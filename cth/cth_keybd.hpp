#pragma once

#include <array>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <Windows.h>

#include <boost/lockfree/queue.hpp>

#include "src/cth_log.hpp"


//-------------------------
//      DEFINITIONS
//-------------------------
namespace cth::keybd {
using namespace std;

namespace dev {
    template<bool Raw>
    struct EventQueueTemplate;
    template<bool Raw>
    struct CallbackEventQueueTemplate;

    inline static vector<EventQueueTemplate<false>*> queues{};
    inline static vector<EventQueueTemplate<true>*> rawQueues{};
    inline static array<mutex, 2> queuesMutex{};
    inline static array<vector<mutex>, 2> queueMutexes{};

    inline static jthread keyboardHookThread{};

    inline static array<int, 255> pressedKeys{};


    template<bool Raw>
    void eraseEventQueue(EventQueueTemplate<Raw>* queue);
    template<bool Raw>
    void addEventQueue(EventQueueTemplate<Raw>* queue);


    inline LRESULT CALLBACK hookFunc(int n_code, WPARAM action, LPARAM l_param);
    inline void hookThreadProc(std::stop_token stop);
    inline void hook();
    inline void unhook();
} //namespace dev


using EventQueue = dev::EventQueueTemplate<false>;
using RawEventQueue = dev::EventQueueTemplate<true>;
using CallbackEventQueue = dev::CallbackEventQueueTemplate<false>;
using RawCallbackEventQueue = dev::CallbackEventQueueTemplate<true>;

using event_t = struct {
    int key; //virtual windows key code
    int action; //0 if released or > 0 for times pressed
};
using raw_event_t = struct {
    KBDLLHOOKSTRUCT key;
    WPARAM action;
};
using callback_t = function<void(int, int)>; //key, press count
using raw_callback_t = function<void(const KBDLLHOOKSTRUCT&, WPARAM)>; //key, action

} //namespace cth::keybd

//-------------------------
//  IMPLEMENTATIONS
//-------------------------
namespace cth::keybd {
using namespace std;

namespace dev {
    template<bool Raw>
    struct EventQueueTemplate {
        using template_event_t = conditional_t<Raw, raw_event_t, event_t>;
        using template_callback_t = conditional_t<Raw, raw_callback_t, callback_t>;

        explicit EventQueueTemplate() {
            lock_guard<mutex> lock(queuesMutex[Raw]);
            if constexpr(Raw) id = queues.size();
            else id = rawQueues.size();

            dev::addEventQueue(this);
        }
        ~EventQueueTemplate() { dev::eraseEventQueue(this); }

        [[nodiscard]] bool empty() const { return eventQueue.empty(); }

        [[nodiscard]] template_event_t pop() {
            CTH_ASSERT(!eventQueue.empty() && "pop: queue is empty");
            if constexpr(!Raw) lastKey = 0;

            lock_guard<mutex> lock(queueMutexes[Raw][id]);

            template_event_t keyEvent = eventQueue.front();
            eventQueue.pop(keyEvent);

            return keyEvent;
        }
        [[nodiscard]] vector<template_event_t> popAll() {
            vector<template_event_t> events{};
            while(!empty()) events.push_back(pop());
            return events;
        }

        void dumpAll() {
            lock_guard<mutex> lock(queueMutexes[Raw][id]);
            while(!empty()) eventQueue.pop();
        }
        void dump() {
            lock_guard<mutex> lock(queueMutexes[Raw][id]);
            eventQueue.pop();
        }

    private:
        void push(template_event_t event) {
            lock_guard<mutex> lock(queueMutexes[Raw][id]);
            if constexpr(!Raw)
                if(event.action > 0) {
                    if(lastKey == event.key) {
                        eventQueue.pop();
                        event.action += 1;
                    } else lastKey = event.key;
                }
            eventQueue.push(event);
        }


        queue<template_event_t> eventQueue{};
        template_callback_t callback{};
        size_t id;

        int lastKey = 0; //count repeated keypresses on normal queue

        friend LRESULT CALLBACK hookFunc(int n_code, WPARAM action, LPARAM l_param);
        template<bool R>
        friend void eraseEventQueue(EventQueueTemplate<R>* queue);
    };
    template<bool Raw>
    struct CallbackEventQueueTemplate {
        using template_event_t = conditional_t<Raw, raw_event_t, event_t>;
        using template_callback_t = conditional_t<Raw, raw_callback_t, callback_t>;

        explicit CallbackEventQueueTemplate(template_callback_t callback_function) : callback(callback_function) {}

        [[nodiscard]] bool empty() const { return eventQueue.empty(); }

        void dumpAll() { eventQueue.dumpAll(); }
        void dump() { eventQueue.dump(); }

        void process() {
            CTH_ASSERT(!eventQueue.empty() && "process: queue is empty");

            template_event_t keybdEvent = eventQueue.pop();
            callback(keybdEvent.key, keybdEvent.action);
        }
        void processAll() { for(const auto& event : eventQueue.popAll()) callback(event.key, event.action); }

        EventQueueTemplate<Raw> eventQueue{};
        template_callback_t callback{};
    };



    LRESULT CALLBACK hookFunc(const int n_code, const WPARAM action, const LPARAM l_param) {
        if(n_code == HC_ACTION) {
            const auto keyStruct = *reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);

            const int vkCode = keyStruct.vkCode;

            if(keyStruct.flags & LLKHF_UP) {
                pressedKeys[vkCode] = 0;

                for(const auto [q, mtx] : views::zip(queues, queueMutexes[0])) {
                    lock_guard<mutex> lock(mtx);
                    q->push(event_t{vkCode, 0});
                }
            } else {
                pressedKeys[vkCode] += 1;
                for(const auto [q, mtx] : views::zip(queues, queueMutexes[0])) {
                    lock_guard<mutex> lock(mtx);
                    q->push(event_t{vkCode, 1});
                }
            }
            for(const auto [q, mtx] : views::zip(rawQueues, queueMutexes[1])) {
                lock_guard<mutex> lock(mtx);
                q->push(raw_event_t{keyStruct, action});
            }
        }
        return CallNextHookEx(nullptr, n_code, action, l_param);
    }
    void hookThreadProc(std::stop_token stop) {

        static HHOOK hookHandle = nullptr;
        hookHandle = SetWindowsHookExW(WH_KEYBOARD_LL, hookFunc, nullptr, 0);

        //IMPLEMENT as soon as CTH_STABLE_ASSERT is here: ABORT(hookHandle == nullptr && "failed to establish key hook");

        MSG msg;
        while(GetMessageW(&msg, nullptr, 0, 0) && !stop.stop_requested()) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        UnhookWindowsHookEx(hookHandle);
    }

    void hook() { keyboardHookThread = std::jthread(hookThreadProc); }
    void unhook() {
        keyboardHookThread.request_stop();
        keyboardHookThread.detach();
    }



    template<bool Raw>
    void addEventQueue(EventQueueTemplate<Raw>* queue) {
        lock_guard<mutex> mtx(queuesMutex[Raw]);

        if(queues.empty() && rawQueues.empty()) hook();

        if constexpr(Raw) rawQueues.push_back(queue);
        else queues.push_back(queue);

    }

    template<bool Raw>
    void eraseEventQueue(EventQueueTemplate<Raw>* queue) {
        CTH_ASSERT(!queues.empty() && !rawQueues.empty() && "cannot remove queue");
        lock_guard<mutex> lock(queuesMutex[Raw]);


        using queue_t = conditional_t<Raw, RawEventQueue, EventQueue>;

        auto* queuesVec = []() {
            if constexpr(Raw) return &rawQueues;
            else return &queues;
        }();

        auto id = queue->id;
        queuesVec->erase(queuesVec->begin() + id);
        for_each(queuesVec->begin() + id - 1, queuesVec->end(), [](queue_t* q) { q->id -= 1; });

        if(queues.empty()) unhook();
    }
} //namespace dev

} //namespace cth::keybd

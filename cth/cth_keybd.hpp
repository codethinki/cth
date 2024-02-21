#pragma once
#include <print>
#include <array>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <Windows.h>

#include "cth_log.hpp"

//-------------------------
//      DEFINITIONS
//-------------------------
namespace cth::win::keybd {
using namespace std;

namespace dev {
    template<bool Raw>
    struct EventQueueTemplate;
    template<bool Raw>
    struct CallbackEventQueueTemplate;

    inline array<int, 255> pressedKeys{};

    inline jthread keyboardHookThread{};
    inline jthread::id threadId{};
    inline uint32_t queueCount = 0;

} //namespace dev


using EventQueue = dev::EventQueueTemplate<false>;
using RawEventQueue = dev::EventQueueTemplate<true>;
using CallbackEventQueue = dev::CallbackEventQueueTemplate<false>;
using RawCallbackEventQueue = dev::CallbackEventQueueTemplate<true>;

using event_t = struct {
    uint32_t key; //virtual windows key code
    uint32_t action; //0 if released or > 0 for times pressed
};
using raw_event_t = struct {
    KBDLLHOOKSTRUCT key;
    WPARAM action;
};
using callback_t = void(uint32_t, uint32_t); //key, action
using raw_callback_t = void(const KBDLLHOOKSTRUCT&, WPARAM); //key, action

} //namespace cth::win::keybd

//-------------------------
//     DECLARATIONS
//-------------------------
namespace cth::win::keybd::dev {
inline LRESULT CALLBACK hookFunc(int n_code, WPARAM action, LPARAM l_param);
inline void threadProc(const std::stop_token& stop);

inline void hook();
inline void unhook();

template<bool Raw>
struct EventQueueTemplate {
    using template_event_t = conditional_t<Raw, raw_event_t, event_t>;
    using template_callback_t = conditional_t<Raw, raw_callback_t, callback_t>;

    explicit EventQueueTemplate();
    ~EventQueueTemplate() { eraseEventQueue(id); }

    [[nodiscard]] bool empty() const { return baseQueue.empty(); }

    [[nodiscard]] template_event_t front() const { return baseQueue.front(); }
    [[nodiscard]] template_event_t pop();
    [[nodiscard]] vector<template_event_t> popQueue();

    void clear();
    void dumpFront();

private:
    void push(template_event_t event);

    queue<template_event_t> baseQueue{};
    function<template_callback_t> callback{};
    size_t id;

    mutex queueMtx{};

    uint32_t lastKey = 0; //count repeated keypresses on normal queue



    static void addEventQueue(EventQueueTemplate* queue);
    static void eraseEventQueue(uint32_t id);

    inline static vector<EventQueueTemplate*> queues{};
    inline static mutex queuesMtx{};



    friend LRESULT CALLBACK hookFunc(int n_code, WPARAM action, LPARAM l_param);
    template<bool R>
    friend void eraseEventQueue(uint32_t id);
    template<bool R>
    friend void addEventQueue(EventQueueTemplate<R>* queue);
};
template<bool Raw>
struct CallbackEventQueueTemplate {
    using template_event_t = conditional_t<Raw, raw_event_t, event_t>;
    using template_callback_t = conditional_t<Raw, raw_callback_t, callback_t>;

    explicit CallbackEventQueueTemplate(function<template_callback_t> callback_function) : callback(callback_function) {}

    [[nodiscard]] bool empty() const { return eventQueue.empty(); }

    void clear() { eventQueue.clear(); }
    void dumpFront() { eventQueue.dumpFront(); }

    void process();
    void processQueue();

    EventQueueTemplate<Raw> eventQueue{};
    function<template_callback_t> callback{};
};



} // namespace cth::win::keybd::dev


//-------------------------
//  IMPLEMENTATIONS
//-------------------------
namespace cth::win::keybd::dev {
using namespace std;



template<bool Raw>
EventQueueTemplate<Raw>::EventQueueTemplate() { addEventQueue(this); }
template<bool Raw>
auto EventQueueTemplate<Raw>::pop()->template_event_t {
    CTH_ERR(!baseQueue.empty(), "pop: baseQueue is empty") throw details.exception();

    lock_guard<mutex> lock(queueMtx);

    if constexpr(!Raw) lastKey = 0;

    template_event_t keyEvent = baseQueue.front();
    baseQueue.pop();

    return keyEvent;

}
template<bool Raw>
auto EventQueueTemplate<Raw>::popQueue()->vector<template_event_t> {
    lock_guard<mutex> lock(queueMtx);

    if constexpr(!Raw) lastKey = 0;


    vector<template_event_t> events{};
    while(!empty()) {
        events.push_back(baseQueue.front());
        baseQueue.pop();
    }
    return events;
}
template<bool Raw>
void EventQueueTemplate<Raw>::clear() {
    lock_guard<mutex> lock(queueMtx);

    while(!empty()) baseQueue.pop();
}
template<bool Raw>
void EventQueueTemplate<Raw>::dumpFront() {
    lock_guard<mutex> lock(queueMtx);

    baseQueue.pop();
}
template<bool Raw>
void EventQueueTemplate<Raw>::push(template_event_t event) {
    lock_guard<mutex> queueLock(queueMtx);

    if constexpr(!Raw)
        if(event.action > 0) {
            if(lastKey == event.key) {
                baseQueue.pop();
                event.action += 1;
            } else lastKey = event.key;
        }
    baseQueue.push(event);
}


template<bool Raw>
void EventQueueTemplate<Raw>::addEventQueue(EventQueueTemplate* queue) {
    if(queueCount++ == 0) hook();
    CTH_STABLE_ERR(keyboardHookThread.get_id() == threadId, "hook thread crashed")
        throw details->exception();

    lock_guard<mutex> lock(queuesMtx);
    queue->id = queues.size();
    queues.push_back(queue);
}
template<bool Raw>
void EventQueueTemplate<Raw>::eraseEventQueue(uint32_t id) {
    CTH_STABLE_ERR(keyboardHookThread.get_id() == threadId, "hook thread crashed") throw details->exception();
    CTH_ERR(queueCount > 0, "no queues active") throw details->exception();

    lock_guard<mutex> lock(queuesMtx);
    queues.erase(queues.begin() + id);
    for_each(queues.begin() + id, queues.end(), [](auto q) { q->id -= 1; });


    if(--queueCount == 0) unhook();
}

template<bool Raw>
void CallbackEventQueueTemplate<Raw>::process() {
    CTH_ERR(!eventQueue.empty(), "process: queue is empty")
        throw details.exception();


    template_event_t keybdEvent = eventQueue.pop();
    callback(keybdEvent.key, keybdEvent.action);
}
template<bool Raw>
void CallbackEventQueueTemplate<Raw>::processQueue() {
    for(auto events = eventQueue.popQueue(); const auto& e : events)
        callback(e.key, e.action);
}


LRESULT CALLBACK hookFunc(const int n_code, const WPARAM action, const LPARAM l_param) {
    if(n_code == HC_ACTION) {
        const auto keyStruct = *reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);
        const bool keyUp = keyStruct.flags & LLKHF_UP;
        const uint32_t vkCode = keyStruct.vkCode;

        pressedKeys[vkCode] = 0;

        unique_lock<mutex> lock(EventQueue::queuesMtx);
        ranges::for_each(EventQueue::queues, [vkCode, keyUp](auto q) { q->push(event_t{vkCode, keyUp ? 0u : 1u}); });
        lock.unlock();

        unique_lock<mutex> rawLock(RawEventQueue::queuesMtx);
        ranges::for_each(RawEventQueue::queues, [keyStruct, action](auto q) { q->push(raw_event_t{keyStruct, action}); });
        rawLock.unlock();
    }
    return CallNextHookEx(nullptr, n_code, action, l_param);
}

void threadProc(const std::stop_token& stop) {
    static HHOOK hookHandle = nullptr;

    try {
        hookHandle = SetWindowsHookExW(WH_KEYBOARD_LL, hookFunc, nullptr, 0);
        CTH_STABLE_ERR(hookHandle != nullptr, "failed to establish key hook") throw details->exception();


        MSG msg;
        while(GetMessageW(&msg, nullptr, 0, 0) && !stop.stop_requested()) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        UnhookWindowsHookEx(hookHandle);
    }
    catch(...) {
        UnhookWindowsHookEx(hookHandle);
        //just return and unhook
    }
}

void hook() {
    keyboardHookThread = std::jthread(threadProc);
    threadId = keyboardHookThread.get_id();
}
void unhook() {
    keyboardHookThread.request_stop();
    keyboardHookThread.detach();
    threadId = jthread::id{};
}

} //namespace cth::win::keybd::dev

#pragma once
#include <array>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <Windows.h>
#include <deque>



import hlc.algorithm;



namespace hlc::keybd {
using namespace std;


using event_t = struct {
    int key; //virtual windows key code
    int action; //0 if released or > 0 for times pressed
};
using raw_event_t = struct {
    KBDLLHOOKSTRUCT key;
    WPARAM action;
};


typedef function<void(int, int)> callback_t; //key, press count
typedef function<void(const KBDLLHOOKSTRUCT&, WPARAM)> raw_callback_t; //key, action

template<bool Raw>
struct EventQueueTemplate;
template<bool Raw>
struct CallbackEventQueueTemplate;



LRESULT CALLBACK hookFunc(int n_code, WPARAM action, LPARAM l_param);
void hook();
void unhook();
template<bool Raw>
void eraseEventQueue(EventQueueTemplate<Raw>* queue);
template<bool Raw>
void addEventQueue(EventQueueTemplate<Raw>* queue);



//TEMP not here
template<bool Raw>
struct EventQueueTemplate {
    typedef conditional_t<Raw, raw_event_t, event_t> template_event_t;
    typedef conditional_t<Raw, raw_callback_t, callback_t> template_callback_t;

    explicit EventQueueTemplate() { keybd::addEventQueue(this); }
    ~EventQueueTemplate() { keybd::eraseEventQueue(this); }

    bool empty() const { return eventQueue.empty(); }

    [[nodiscard]] template_event_t pop() {
        ABORT(!eventQueue.empty() && "pop: queue is empty");
        if constexpr(!Raw) lastKey = 0;

        template_event_t keyEvent;
        eventQueue.unsynchronized_pop(keyEvent);

        return keyEvent;
    }
    [[nodiscard]] vector<template_event_t> popAll() {
        vector<template_event_t> events{};
        while(!empty()) events.push_back(pop());
        return events;
    }

    void dump() { while(!empty()) eventQueue.unsynchronized_pop(); }
    void dumpLast() { eventQueue.unsynchronized_pop(); }

    void push(template_event_t event) {
        if constexpr(!Raw)
            if(event.action > 0) {
                if(lastKey == event.key) {
                    eventQueue.unsynchronized_pop(event);
                    event.action += 1;
                } else lastKey = event.key;
            }
        eventQueue.unsynchronized_push(event);
    }

private:
    //friend LRESULT CALLBACK hookFunc(int n_code, WPARAM action, LPARAM l_param);


    boost::lockfree::queue<template_event_t> eventQueue{5};
    template_callback_t callback{};

    int lastKey = 0; //count repeated keypresses on normal queue
};

//TEMP not here
template<bool Raw>
struct CallbackEventQueueTemplate {
    typedef conditional_t<Raw, raw_event_t, event_t> template_event_t;
    typedef conditional_t<Raw, raw_callback_t, callback_t> template_callback_t;

    explicit CallbackEventQueueTemplate(template_callback_t callback_function) : callback(callback_function) {}

    bool empty() const { return eventQueue.empty(); }

    void dump() { while(!eventQueue.empty()) eventQueue.dump(); }
    void dumpLast() { eventQueue.dumpLast(); }

    void process() {
        ABORT(!eventQueue.empty() && "process: queue is empty");

        template_event_t keybdEvent = eventQueue.pop();
        callback(keybdEvent.key, keybdEvent.action);
    }
    void processAll() { while(!empty()) process(); }

    EventQueueTemplate<Raw> eventQueue{};
    template_callback_t callback{};
};

typedef EventQueueTemplate<false> EventQueue;
typedef EventQueueTemplate<true> RawEventQueue;
typedef CallbackEventQueueTemplate<false> CallbackEventQueue;
typedef CallbackEventQueueTemplate<true> RawCallbackEventQueue;


vector<EventQueue*> queues{};
vector<RawEventQueue*> rawQueues{};
mutex queuesMtx{};

thread keyboardHookThread{};
atomic<bool> threadStop = true;
array<int, 255> pressedKeys{};


//TEMP not here
LRESULT CALLBACK hookFunc(const int n_code, const WPARAM action, const LPARAM l_param) {
    if(n_code == HC_ACTION) {
        const auto keyStruct = *reinterpret_cast<KBDLLHOOKSTRUCT*>(l_param);

        lock_guard<mutex> mtx(queuesMtx);
        const int vkCode = keyStruct.vkCode;

        if(keyStruct.flags & LLKHF_UP) {
            pressedKeys[vkCode] = 0;
            for(const auto& queue : queues) queue->push(event_t{vkCode, 0});
        } else {
            pressedKeys[vkCode] += 1;
            for(const auto& queue : queues) queue->push(event_t{vkCode, 1});
        }

        for(const auto& queue : rawQueues) queue->push(raw_event_t{keyStruct, action});
    }
    return CallNextHookEx(nullptr, n_code, action, l_param);
}
//TEMP not here
void hookThreadProc() {
    static HHOOK hookHandle = nullptr;
    hookHandle = SetWindowsHookExW(WH_KEYBOARD_LL, hookFunc, nullptr, 0);

    //ABORT(hookHandle == nullptr && "failed to establish key hook");

    threadStop = false;
    MSG msg;
    while(GetMessageW(&msg, nullptr, 0, 0) && !threadStop) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(hookHandle);
}
//TEMP not here
void hook() { keyboardHookThread = std::thread(hookThreadProc); }
//TEMP not here
void unhook() {
    threadStop = true;
    keyboardHookThread.detach();
}

//TEMP not here
template<bool Raw>
void addEventQueue(EventQueueTemplate<Raw>* queue) {
    lock_guard<mutex> mtx(queuesMtx);

    if(queues.empty() && rawQueues.empty()) hook();

    if constexpr(Raw) rawQueues.push_back(queue);
    else queues.push_back(queue);

}
//TEMP not here
template<bool Raw>
void eraseEventQueue(EventQueueTemplate<Raw>* queue) {
    lock_guard<mutex> mtx(queuesMtx);
    ABORT(!queues.empty() && !rawQueues.empty() && "cannot remove queue");


    if constexpr(Raw) std::erase(rawQueues, queue);
    else std::erase(queues, queue);

    if(queues.empty()) unhook();
}

}

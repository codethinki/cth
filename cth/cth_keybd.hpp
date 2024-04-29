#pragma once
#include <array>
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <Windows.h>

#include "cth_log.hpp"


namespace cth::win::keybd {


namespace dev {
    template<bool Raw>
    struct EventQueueTemplate;
    template<bool Raw>
    struct CallbackEventQueueTemplate;

    inline std::array<int, 255> pressedKeys{};

    inline std::jthread keyboardHookThread{};
    inline std::jthread::id threadId{};
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
    using template_event_t = std::conditional_t<Raw, raw_event_t, event_t>;
    using template_callback_t = std::conditional_t<Raw, raw_callback_t, callback_t>;

    explicit EventQueueTemplate();
    ~EventQueueTemplate() { eraseEventQueue(_id, 1); }

    [[nodiscard]] bool empty() const { return _baseQueue.empty(); }

    [[nodiscard]] template_event_t front() const { return _baseQueue.front(); }
    [[nodiscard]] template_event_t pop();
    [[nodiscard]] std::vector<template_event_t> popQueue();

    void clear();
    void dumpFront();

private:
    void push(template_event_t event);

    std::queue<template_event_t> _baseQueue{};
    std::function<template_callback_t> _callback{};
    size_t _id;

    std::mutex _queueMtx{};

    uint32_t _lastKey = 0; //count repeated keypresses on normal queue



    static void addEventQueue(EventQueueTemplate* queue);
    static void eraseEventQueue(size_t id);
    static int eraseEventQueue(size_t id, int error_code);

    inline static std::vector<EventQueueTemplate*> _queues{};
    inline static std::mutex _queuesMtx{};



    friend LRESULT CALLBACK hookFunc(int n_code, WPARAM action, LPARAM l_param);
    template<bool R>
    friend void eraseEventQueue(uint32_t id);
    template<bool R>
    friend void addEventQueue(EventQueueTemplate<R>* queue);
};
template<bool Raw>
struct CallbackEventQueueTemplate {
    using template_event_t = std::conditional_t<Raw, raw_event_t, event_t>;
    using template_callback_t = std::conditional_t<Raw, raw_callback_t, callback_t>;

    explicit CallbackEventQueueTemplate(std::function<template_callback_t> callback_function) : callback(callback_function) {}

    [[nodiscard]] bool empty() const { return eventQueue.empty(); }

    void clear() { eventQueue.clear(); }
    void dumpFront() { eventQueue.dumpFront(); }

    void process();
    void processQueue();

    EventQueueTemplate<Raw> eventQueue{};
    std::function<template_callback_t> callback{};
};



} // namespace cth::win::keybd::dev


//-------------------------
//  IMPLEMENTATIONS
//-------------------------
namespace cth::win::keybd::dev {



template<bool Raw>
EventQueueTemplate<Raw>::EventQueueTemplate() { addEventQueue(this); }
template<bool Raw>
auto EventQueueTemplate<Raw>::pop()->template_event_t {
    CTH_ERR(_baseQueue.empty(), "pop: baseQueue is empty") throw details->exception();

    std::lock_guard lock(_queueMtx);

    if constexpr(!Raw) _lastKey = 0;

    template_event_t keyEvent = _baseQueue.front();
    _baseQueue.pop();

    return keyEvent;

}
template<bool Raw>
auto EventQueueTemplate<Raw>::popQueue()->std::vector<template_event_t> {
    std::lock_guard<std::mutex> lock(_queueMtx);

    if constexpr(!Raw) _lastKey = 0;


    std::vector<template_event_t> events{};
    while(!empty()) {
        events.push_back(_baseQueue.front());
        _baseQueue.pop();
    }
    return events;
}
template<bool Raw>
void EventQueueTemplate<Raw>::clear() {
    std::lock_guard lock(_queueMtx);

    while(!empty()) _baseQueue.pop();
}
template<bool Raw>
void EventQueueTemplate<Raw>::dumpFront() {
    std::lock_guard<std::mutex> lock(_queueMtx);

    _baseQueue.pop();
}
template<bool Raw>
void EventQueueTemplate<Raw>::push(template_event_t event) {
    std::lock_guard<std::mutex> queueLock(_queueMtx);

    if constexpr(!Raw)
        if(event.action > 0) {
            if(_lastKey == event.key) {
                _baseQueue.pop();
                event.action += 1;
            } else _lastKey = event.key;
        }
    _baseQueue.push(event);
}


template<bool Raw>
void EventQueueTemplate<Raw>::addEventQueue(EventQueueTemplate* queue) {
    if(queueCount++ == 0) hook();
    CTH_STABLE_ERR(keyboardHookThread.get_id() != threadId, "hook thread crashed")
        throw details->exception();

    std::lock_guard<std::mutex> lock(_queuesMtx);
    queue->_id = _queues.size();
    _queues.push_back(queue);
}
template<bool Raw>
void EventQueueTemplate<Raw>::eraseEventQueue(size_t id) {
    CTH_STABLE_ERR(keyboardHookThread.get_id() != threadId, "hook thread crashed") throw details->exception();
    CTH_ERR(queueCount <= 0, "no queues active") throw details->exception();

    std::lock_guard<std::mutex> lock(_queuesMtx);
    _queues.erase(_queues.begin() + id);
    for_each(_queues.begin() + id, _queues.end(), [](auto q) { q->id -= 1; });


    if(--queueCount == 0) unhook();
}
template<bool Raw>
int EventQueueTemplate<Raw>::eraseEventQueue(size_t id, const int error_code) {
    if(keyboardHookThread.get_id() != threadId) return error_code;
    if(queueCount <= 0) return error_code;

    std::lock_guard lock(_queuesMtx);
    _queues.erase(_queues.begin() + id);
    std::for_each(_queues.begin() + id, _queues.end(), [](auto q) { q->_id -= 1; });


    if(--queueCount == 0) unhook();
    return EXIT_SUCCESS;
}

template<bool Raw>
void CallbackEventQueueTemplate<Raw>::process() {
    CTH_ERR(eventQueue.empty(), "process: queue is empty")
        throw details->exception();


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

        std::unique_lock lock(EventQueue::_queuesMtx);
        std::ranges::for_each(EventQueue::_queues, [vkCode, keyUp](auto q) { q->push(event_t{vkCode, keyUp ? 0u : 1u}); });
        lock.unlock();

        std::unique_lock rawLock(RawEventQueue::_queuesMtx);
        std::ranges::for_each(RawEventQueue::_queues, [keyStruct, action](auto q) { q->push(raw_event_t{keyStruct, action}); });
        rawLock.unlock();
    }
    return CallNextHookEx(nullptr, n_code, action, l_param);
}

void threadProc(const std::stop_token& stop) {
    static HHOOK hookHandle = nullptr;

    try {
        hookHandle = SetWindowsHookExW(WH_KEYBOARD_LL, hookFunc, nullptr, 0);
        CTH_STABLE_ERR(hookHandle == nullptr, "failed to establish key hook") throw details->exception();


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
    threadId = std::jthread::id{};
}

} //namespace cth::win::keybd::dev

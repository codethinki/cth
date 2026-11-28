#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/ptr/ref_wrapper.hpp>
#include <cth/types.hpp>

#include <atomic>
#include <coroutine>
#include <cstdint>


namespace cth::co {
/**
 * counter that only rises, coroutines wait for a value of it
 * @details
 * - a wait resumes its caller on the context of the caller once `value()` reaches the value of the wait
 * - `set()` posts the resumes, it resumes no caller inline
 * - lock free, no allocation per wait: the awaiter in the frame of the caller holds the node of the wait
 * - thread safe: `wait()` and `value()` from any thread, `set()` from one thread at a time
 */
class counter_event {
    struct node;

public:
    class awaiter;

    /**
     * constructs the event with the start value
     * @param value start value
     */
    constexpr explicit counter_event(u64 value = 0) noexcept : _value{value} {}

    /**
     * @details waits for the end of a running `set()` and `awaiter::await_suspend()`
     * @pre no pending wait
     */
    ~counter_event();

    /**
     * raises the value, posts the resume of every reached wait to the context of its caller
     * @param value new value
     * @pre `value > value()`
     * @pre no concurrent `set()`
     * @attention a post that throws terminates the program
     */
    void set(u64 value) noexcept;

    /**
     * waits until `value()` reaches the value
     * @details `value() >= value` -> no suspension
     * @param value to wait for
     * @return awaiter for a `this_coro` caller, `co_await` it
     */
    [[nodiscard]] constexpr awaiter wait(u64 value) const noexcept;

    [[nodiscard]] u64 value() const noexcept { return _value.load(std::memory_order_acquire); }

private:
    /**
     * pushes the node onto the stack of the suspended waits
     */
    void push(node&) const noexcept;

    /**
     * takes the suspended waits, posts the resume of the reached ones, pushes back the others
     * @details a `set()` during a pass -> another pass
     * @attention a post that throws terminates the program
     */
    void drain() const noexcept;

    /**
     * waits until no `set()` and no `awaiter::await_suspend()` runs
     */
    void wait_in_flight() const noexcept;

    std::atomic<u64> _value;
    /** lock free stack of the suspended waits, `nullptr` -> no pending wait */
    mutable std::atomic<node*> _head{nullptr};
    /** running `set()` and `awaiter::await_suspend()` calls, the destructor and the moves wait for `0` */
    mutable std::atomic<u32> _inFlight{0};

public:
    counter_event(counter_event const& other) = delete;
    counter_event& operator=(counter_event const& other) = delete;
    /**
     * copies the value of `other`
     * @details waits for the end of a running `set()` and `awaiter::await_suspend()` on `other`
     * @pre no pending wait on `other`
     */
    counter_event(counter_event&& other) noexcept;
    /**
     * copies the value of `other`
     * @details waits for the end of a running `set()` and `awaiter::await_suspend()` on both events
     * @pre no pending wait on both events
     */
    counter_event& operator=(counter_event&& other) noexcept;
};
}

namespace cth::co {
/**
 * suspended wait of `counter_event::wait()`, a node of the stack of the event
 */
struct counter_event::node {
    u64 value; ///< to wait for
    std::coroutine_handle<> caller{};
    co::context const* context = nullptr; ///< of the caller, the resume is posted to it
    node* next = nullptr;
};

/**
 * awaiter of `counter_event::wait()`
 * @details holds the node of the wait: the frame of the caller holds it, no allocation
 */
class counter_event::awaiter : public this_coro_awaiter_base {
public:
    constexpr awaiter(counter_event const& event, u64 value) noexcept :
        _event{event},
        _node{.value = value} {}

    /**
     * checks the value without a suspension
     */
    [[nodiscard]] bool await_ready() const noexcept { return _event->value() >= _node.value; }

    /**
     * pushes the wait onto the event, the event resumes the caller on its context
     * @details the value is reached during the push -> resumes the reached waits itself
     * @param caller coroutine of the caller
     */
    template<this_coro_compatible Caller>
    void await_suspend(std::coroutine_handle<Caller> caller) noexcept {
        _node.caller = caller;
        _node.context = &caller.promise().context();

        // after the push a drain can resume the caller at once: nothing touches the frame afterwards
        auto const& event = *_event;
        auto const value = _node.value;
        // relaxed: the release of the push orders it before the resume, a destruction comes after the resume
        event._inFlight.fetch_add(1, std::memory_order_relaxed);
        event.push(_node);

        // a set() between await_ready() and the push found no node to resume
        if(event.value() >= value)
            event.drain();

        // release, the last access: the resumed caller can destroy the event, the destructor waits for it
        event._inFlight.fetch_sub(1, std::memory_order_release);
    }

    constexpr void await_resume() const noexcept {}

private:
    cth::ref_wrapper<counter_event const> _event;
    node _node;
};

constexpr auto counter_event::wait(u64 value) const noexcept -> awaiter { return awaiter{*this, value}; }
}

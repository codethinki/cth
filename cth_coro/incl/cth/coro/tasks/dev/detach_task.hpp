#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/simple_counting_scope.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/ptr/ref_wrapper.hpp>

#include <coroutine>
#include <exception>


namespace cth::co::dev {
/**
 * final awaiter of `detach_task`
 * @details releases the coroutine from its owner, then destroys it
 */
struct final_detach_awaiter {
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    /**
     * releases the coroutine from its owner, then destroys it
     * @param h coroutine of the detached task
     */
    template<class Promise>
    void await_suspend(std::coroutine_handle<Promise> h) noexcept {
        // before the destroy: the awaiter lives in the coroutine frame
        h.promise().owner().release(h);
        // the last action: the destruction leaves the scope at its end, see detach_scope_guard
        h.destroy();
    }

    constexpr void await_resume() const noexcept {}
};

// a first base: the promise destructor body runs before the destruction of the bases and the parameters
/**
 * first base of `detach_promise`, the task is in the scope for its lifetime
 * @details
 * - enters the scope on construction, leaves it on destruction
 * - the first base: destroyed after the other parts of the promise
 */
struct detach_scope_guard {
    explicit detach_scope_guard(co::simple_counting_scope& scope) noexcept : _scope{scope} { scope.enter(); }

    // the last access to the scope: the deallocation of the frame after it does not touch the scope
    ~detach_scope_guard() { _scope->leave(); }

private:
    cth::ref_wrapper<co::simple_counting_scope> _scope;

public:
    detach_scope_guard(detach_scope_guard const& other) = delete;
    detach_scope_guard(detach_scope_guard&& other) = delete;
    detach_scope_guard& operator=(detach_scope_guard const& other) = delete;
    detach_scope_guard& operator=(detach_scope_guard&& other) = delete;
};

template<payload Pyld>
struct detach_promise;

/**
 * return type of the coroutine of `executor::detach()`
 * @details owns nothing: the scheduler of the detach owns the coroutine, it destroys itself at its end
 * @tparam Pyld payload of the detached task
 */
template<payload Pyld>
struct detach_task {
    using promise_type = detach_promise<Pyld>;
};

/**
 * promise of `detach_task`
 * @details
 * - the awaited task attaches to it: a `this_coro` coroutine
 * - starts at once, `final_detach_awaiter` ends it
 * - the scheduler of the detach owns the coroutine until it ends, see `context::detach()`
 * - leaves the scope after the other parts of the coroutine are destroyed, see `detach_scope_guard`
 * @tparam Pyld payload of the detached task
 */
template<payload Pyld>
struct detach_promise : detach_scope_guard, this_coro_promise_base<Pyld> {
    /**
     * constructs from the coroutine parameters, enters the scope
     * @details calls `context::detach()` with the coroutine
     * @param scope the task enters
     * @param payload of the detached task
     * @param context the task runs on, its scheduler owns the coroutine
     */
    template<class... Args>
    explicit detach_promise(
        co::simple_counting_scope& scope,
        Pyld const& payload,
        co::context const& context,
        Args&&...
    ) :
        detach_scope_guard{scope},
        this_coro_promise_base<Pyld>{context, payload},
        // a throw: detach_scope_guard leaves the scope
        _owner{context.detach(std::coroutine_handle<detach_promise>::from_promise(*this))} {}

    [[nodiscard]] detach_task<Pyld> get_return_object() noexcept { return {}; }

    [[nodiscard]] constexpr std::suspend_never initial_suspend() noexcept { return {}; }

    [[nodiscard]] constexpr final_detach_awaiter final_suspend() noexcept { return {}; }

    constexpr void return_void() noexcept {}

    /**
     * a detached task has no caller for the exception
     */
    [[noreturn]] void unhandled_exception() noexcept { std::terminate(); }

    /**
     * gives the owner of the coroutine: the scheduler of the detach
     */
    [[nodiscard]] co::context const& owner() const noexcept { return *_owner; }

private:
    cth::ref_wrapper<co::context const> _owner;
};
}

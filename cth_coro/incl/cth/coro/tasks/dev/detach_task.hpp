#pragma once
#include "cth/coro/simple_counting_scope.hpp"
#include "cth/coro/tasks/promises/this_coro_promise_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/ptr/ref_wrapper.hpp>

#include <coroutine>
#include <exception>


namespace cth::co::dev {
/**
 * final awaiter of `detach_task`
 * @details destroys the coroutine, then leaves the scope
 */
struct final_detach_awaiter {
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    /**
     * destroys the coroutine, then leaves the scope of the task
     * @param h coroutine of the detached task
     */
    template<class Promise>
    void await_suspend(std::coroutine_handle<Promise> h) noexcept {
        // before the destroy: the awaiter lives in the coroutine frame
        co::simple_counting_scope& owner = h.promise().owner();
        // the frame, the awaited task in it and everything they hold
        h.destroy();
        // the last action: the frame is gone
        owner.leave();
    }

    constexpr void await_resume() const noexcept {}
};

template<payload Pyld>
struct detach_promise;

/**
 * return type of the coroutine of `executor::detach()`
 * @details owns nothing, the coroutine destroys itself at its end
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
 * @tparam Pyld payload of the detached task
 */
template<payload Pyld>
struct detach_promise : this_coro_promise_base<Pyld> {
    /**
     * constructs from the coroutine parameters, enters the scope
     * @details defined in executor.hpp, the executor is incomplete here
     * @param scope the task enters
     * @param payload of the detached task
     * @param self executor that detaches the task, the task runs on its context
     */
    template<class... Args>
    explicit detach_promise(co::simple_counting_scope& scope, Pyld const& payload, executor const& self, Args&&...);

    [[nodiscard]] detach_task<Pyld> get_return_object() noexcept { return {}; }

    [[nodiscard]] constexpr std::suspend_never initial_suspend() noexcept { return {}; }

    [[nodiscard]] constexpr final_detach_awaiter final_suspend() noexcept { return {}; }

    constexpr void return_void() noexcept {}

    /**
     * a detached task has no caller for the exception
     */
    [[noreturn]] void unhandled_exception() noexcept { std::terminate(); }

    [[nodiscard]] co::simple_counting_scope& owner() const noexcept { return *_scope; }

private:
    cth::ref_wrapper<co::simple_counting_scope> _scope;
};
}

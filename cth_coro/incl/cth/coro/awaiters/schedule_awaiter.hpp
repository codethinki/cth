#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <coroutine>


namespace cth::co {
/**
 * moves the caller onto a target `context`
 * @details a `this_coro` caller runs on the target afterwards, its payload stays
 */
struct schedule_awaiter : this_coro_awaiter_base {
public:
    constexpr schedule_awaiter(co::context const& target) : _target{target} {}

    /**
     * never ready, `await_suspend()` decides
     */
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    /**
     * rebinds a `this_coro` caller to the target and posts the caller to the target
     * @details `context::owns_thread()` is not enough: a scheduler also owns the threads of its strand jobs
     * @param h coroutine of the caller
     * @return `false` -> `context::current()` is the target, the caller continues without a post
     */
    template<promise Promise>
    bool await_suspend(std::coroutine_handle<Promise> h) {
        if constexpr(this_coro_compatible<Promise>)
            h.promise().rebind_context(_target);

        if(context::current() == &_target)
            return false;

        _target.post([h]() { h.resume(); });
        return true;
    }

    constexpr void await_resume() const noexcept {}

private:
    co::context const& _target;
};
}

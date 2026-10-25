#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <coroutine>


namespace cth::co {
/**
 * switches execution onto a target @ref context and stays there
 * @details a this_coro caller runs on the target afterwards, its payload stays
 */
struct schedule_awaiter : this_coro_awaiter_base {
public:
    constexpr schedule_awaiter(co::context const& target) : _target{target} {}

    /**
     * @return false, @ref await_suspend() decides
     */
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    /**
     * rebinds a this_coro caller to the target and posts the caller onto the target
     * @details owns_thread() is not enough: a scheduler also owns the threads that run the jobs of its strands
     * @return false if @ref current_context() is the target, the caller continues without a post
     */
    template<promise Promise>
    bool await_suspend(std::coroutine_handle<Promise> h) {
        if constexpr(this_coro_compatible<Promise>)
            h.promise().rebind_context(_target);

        if(co::current_context() == &_target)
            return false;

        _target.post([h]() { h.resume(); });
        return true;
    }

    constexpr void await_resume() const noexcept {}

private:
    co::context const& _target;
};
}

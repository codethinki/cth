#pragma once
#include "cth/coro/scheduler.hpp"

#include <coroutine>


namespace cth::co::dev {
/**
 * final awaiter, continues the parent on its scheduler
 * @details transfers to the parent if its scheduler owns the thread, else posts the resume onto it,
 *  runs after co_return and after an exception
 * @attention the post can throw inside noexcept: terminates
 */
struct affine_final_promise_awaiter {
    [[nodiscard]] constexpr bool await_ready() noexcept { return false; }

    /**
     * @param h of the ending task, its promise derives @ref affine_promise_base
     * @return parent if its scheduler owns the thread, else std::noop_coroutine()
     */
    template<class Promise>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> h) noexcept {
        auto const continuation = h.promise().continuation;
        auto const& target = *h.promise().parentScheduler;
        if(target.owns_thread())
            return continuation;
        target.post([continuation] { continuation.resume(); }); // nothing touches the frame after the post
        return std::noop_coroutine();
    }

    constexpr void await_resume() noexcept {}
};
}

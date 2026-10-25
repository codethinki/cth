#pragma once
#include "cth/coro/context.hpp"

#include <coroutine>


namespace cth::co::dev {
/**
 * final awaiter, continues the caller on its context
 * @details transfers to the caller if @ref context::current() is the context of the caller, else posts the resume
 * @details runs after co_return and after an exception
 * @attention the post can throw inside noexcept: terminates
 */
struct affine_final_promise_awaiter {
    [[nodiscard]] constexpr bool await_ready() noexcept { return false; }

    /**
     * transfers to the caller or posts the resume to the context of the caller
     * @param h coroutine of the task that ends, its promise derives @ref affine_promise
     * @return caller if @ref context::current() is the context of the caller, else std::noop_coroutine()
     */
    template<class Promise>
    std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> h) noexcept {
        auto const continuation = h.promise().continuation;
        auto const* target = h.promise().callerContext;
        if(context::current() == target)
            return continuation;
        target->post([continuation] { continuation.resume(); }); // nothing touches the frame after the post
        return std::noop_coroutine();
    }

    constexpr void await_resume() noexcept {}
};
}

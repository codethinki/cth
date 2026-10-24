#pragma once
#include "cth/coro/scheduler.hpp"

#include <coroutine>


namespace cth::co::dev {
/**
 * final awaiter of @ref spawn_promise, resumes the awaiting coroutine on its own scheduler
 * @details transfers directly if the scheduler of the caller owns the thread, else posts the resume onto it,
 *  a caller without a known scheduler continues where the spawned coroutine ended
 * @attention terminates if the post throws, the final awaiter is noexcept
 */
struct spawn_final_awaiter {
    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }

    template<class Promise>
    [[nodiscard]] auto await_suspend(std::coroutine_handle<Promise> h) const noexcept -> std::coroutine_handle<> {
        auto const continuation = h.promise().continuation;
        if(!continuation)
            return std::noop_coroutine();

        auto const* caller = h.promise().caller;
        if(caller == nullptr || caller->owns_thread())
            return continuation;

        // the caller may destroy this coroutine as soon as it resumes, nothing touches the frame after the post
        caller->post([continuation] { continuation.resume(); });
        return std::noop_coroutine();
    }

    constexpr void await_resume() const noexcept {}
};
}

#pragma once
#include "cth/coro/context.hpp"

#include <coroutine>


namespace cth::co::dev {
/**
 * final awaiter of @ref spawn_promise, resumes the awaiting coroutine on its own context
 * @details transfers directly if @ref context::current() is the context of the caller, else posts the resume onto it,
 *  a caller without a known context continues where the spawned coroutine ended
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
        if(caller == nullptr || context::current() == caller)
            return continuation;

        // the caller may destroy this coroutine as soon as it resumes, nothing touches the frame after the post
        caller->post([continuation] { continuation.resume(); });
        return std::noop_coroutine();
    }

    constexpr void await_resume() const noexcept {}
};
}

#pragma once
#include "cth/coro/utility/concepts.hpp"

namespace cth::co {
/**
 * final awaiter of `sync_task`, signals the threads that wait
 * @details resumes the continuation if the sync task has one
 */
struct final_sync_awaiter {
    constexpr bool await_ready() noexcept { return false; }

    /**
     * signals the promise, then transfers to the continuation
     * @details reads the continuation before the signal: a thread that waits can destroy the coroutine after it
     * @param h coroutine of the sync task
     * @return continuation, `std::noop_coroutine()` if there is none
     */
    template<sync_promise_type Promise>
    auto await_suspend(std::coroutine_handle<Promise> h) noexcept -> std::coroutine_handle<> {
        auto co = h.promise().continuation;

        h.promise().signal();

        if(co)
            return co;
        return std::noop_coroutine();
    }

    constexpr void await_resume() noexcept {}
};

}

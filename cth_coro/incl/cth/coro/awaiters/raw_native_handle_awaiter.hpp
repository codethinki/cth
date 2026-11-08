#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/awaiters/base/wait_awaiter_base.hpp"

#include <cth/os/wait.hpp>

#include <coroutine>


namespace cth::co {
/**
 * waits until the handle is signaled, resumes the caller on the context
 * @details awaitable in any coroutine
 * @details `this_coro` coroutines get it from `this_coro::await_handle()` and `co_await` on a native handle waitable
 * @details a failed wait throws in the caller
 */
struct raw_native_handle_awaiter : wait_awaiter_base {
    constexpr raw_native_handle_awaiter(co::context const& ctx, os::native_handle_t h) : context{ctx}, handle{h} {}

    co::context const& context;
    os::native_handle_t handle;

    /**
     * checks the handle without a wait
     * @note consumes the signal of an auto reset handle
     */
    [[nodiscard]] bool await_ready() const { return os::check_ready(handle); }
    /**
     * resumes the caller on the context once the handle is signaled
     * @param h coroutine of the caller
     */
    void await_suspend(std::coroutine_handle<> h) { context.await(handle, resume(h)); }
};
}

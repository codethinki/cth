#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/awaiters/base/wait_awaiter_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/os/wait.hpp>

#include <coroutine>


namespace cth::co {
/**
 * base of awaiters that wait on a native handle
 * @details resumes the caller on its context, a failed wait throws in the caller
 * @attention a derived `await_resume()` calls `wait_awaiter_base::await_resume()` first
 */
struct native_handle_awaiter_base : this_coro_awaiter_base, wait_awaiter_base {
    constexpr explicit native_handle_awaiter_base(os::native_handle_t handle) : _handle{handle} {}

    /**
     * checks the handle without a wait
     * @note consumes the signal of an auto reset handle
     */
    [[nodiscard]] bool await_ready() const { return os::check_ready(_handle); }

    /**
     * resumes the caller on its context once the handle is signaled
     * @param caller coroutine of the caller
     */
    template<this_coro_compatible Caller>
    void await_suspend(std::coroutine_handle<Caller> caller) {
        caller.promise().context().await(_handle, resume(caller));
    }

private:
    os::native_handle_t _handle;
};
}

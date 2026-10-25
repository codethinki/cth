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
 * @attention a derived await_resume() calls @ref wait_awaiter_base::await_resume() first
 */
struct native_handle_awaiter_base : this_coro_awaiter_base, wait_awaiter_base {
    constexpr explicit native_handle_awaiter_base(os::native_handle_t handle) : _handle{handle} {}

    [[nodiscard]] bool await_ready() const { return os::check_ready(_handle); }

    /**
     * resumes the caller on its context once the handle is signaled
     * @param caller this_coro coroutine that awaits the handle
     */
    template<class Caller>
    void await_suspend(std::coroutine_handle<Caller> caller) {
        static_assert(this_coro_compatible<Caller>, "native handle awaiters need a this_coro caller");
        caller.promise().context().await(_handle, resume(caller));
    }

private:
    os::native_handle_t _handle;
};
}

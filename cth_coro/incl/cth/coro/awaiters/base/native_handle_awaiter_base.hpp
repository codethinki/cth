#pragma once
#include "cth/coro/scheduler.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/os/wait.hpp>

#include <coroutine>


namespace cth::co {
/**
 * base of awaiters that wait on a native handle
 * @details resumes the caller on its scheduler, derived awaiters add await_resume()
 */
struct native_handle_awaiter_base : this_coro_awaiter_base {
    explicit native_handle_awaiter_base(os::native_handle_t handle) : _handle{handle} {}

    [[nodiscard]] bool await_ready() const { return os::check_ready(_handle); }

    /**
     * resumes the caller on its scheduler once the handle is signaled
     * @param caller this_coro coroutine that awaits the handle
     */
    template<class Caller>
    void await_suspend(std::coroutine_handle<Caller> caller) const {
        static_assert(this_coro_compatible<Caller>, "native handle awaiters need a this_coro caller");
        caller.promise().scheduler().await(_handle, [caller] { caller.resume(); });
    }

private:
    os::native_handle_t _handle;
};
}

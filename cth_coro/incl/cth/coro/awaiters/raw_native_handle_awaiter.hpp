#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/awaiters/base/wait_awaiter_base.hpp"

#include <cth/os/wait.hpp>

#include <coroutine>


namespace cth::co {
/**
 * waits until the handle is signaled, resumes the caller on the context
 * @details a failed wait throws in the caller
 */
struct raw_native_handle_awaiter : wait_awaiter_base {
    constexpr raw_native_handle_awaiter(co::context const& ctx, os::native_handle_t h) : context{ctx}, handle{h} {}

    co::context const& context;
    os::native_handle_t handle;

    [[nodiscard]] bool await_ready() const { return os::check_ready(handle); }
    void await_suspend(std::coroutine_handle<> h) { context.await(handle, resume(h)); }
};
}

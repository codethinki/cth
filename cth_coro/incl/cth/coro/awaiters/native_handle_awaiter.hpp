#pragma once

#include "cth/coro/scheduler.hpp"

#include <coroutine>

#include <cth/coro/os/native_handle.hpp>

namespace cth::co {
struct native_handle_awaiter {
    scheduler const& scheduler;
    os::native_handle_t handle;

    [[nodiscard]] bool await_ready() const { return os::check_ready(handle); }
    void await_suspend(std::coroutine_handle<> h) const { scheduler.await(handle, [h] { h.resume(); }); }
    static constexpr void await_resume() {}
};
}

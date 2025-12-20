#pragma once
#include "cth/coro/scheduler.hpp"

#include <coroutine>


namespace cth::co {
struct schedule_awaiter {
    scheduler& scheduler;

    [[nodiscard]] bool await_ready() const noexcept { return scheduler.owns_thread(); }

    void await_suspend(std::coroutine_handle<> h) const { scheduler.post([h]() { h.resume(); }); }

    void await_resume() const noexcept {}
};
}

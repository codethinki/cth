#pragma once
#include "cth/coro/scheduler.hpp"
#include "cth/coro/tasks/awaiters/dev/this_coro_awaiter_base.hpp"

#include <cth/os/wait.hpp>


namespace cth::co {
template<this_coro::scheduler_payload Pyld = this_coro::default_payload>
struct native_handle_awaiter_base : dev::this_coro_awaiter_base<Pyld> {
    explicit native_handle_awaiter_base(os::native_handle_t handle) : _handle{handle} {}

    [[nodiscard]] bool await_ready() const { return os::check_ready(_handle); }
    void await_suspend(std::coroutine_handle<> h) const {
        this->payload().scheduler().await(_handle, [h] { h.resume(); });
    }

private:
    os::native_handle_t _handle;
};
}

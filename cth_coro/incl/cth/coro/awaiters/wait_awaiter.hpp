#pragma once
#include "cth/coro/scheduler.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <chrono>
#include <coroutine>


namespace cth::co {
struct wait_awaiter {

    std::chrono::steady_clock::time_point timePoint;
    co::scheduler const& scheduler;

    [[nodiscard]] constexpr bool await_ready() const noexcept {
        return std::chrono::steady_clock::now() >= timePoint;
    }

    void await_suspend(std::coroutine_handle<> h) const {
        scheduler.await(timePoint, [h]() { h.resume(); });
    }

    constexpr void await_resume() const noexcept {}
};

}

#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/chrono.hpp>

#include <coroutine>


namespace cth::co {
struct wait_awaiter {

    chrono::time_point_t timePoint;
    co::context const& context;

    [[nodiscard]] bool await_ready() const noexcept {
        return chrono::clock_t::now() >= timePoint;
    }

    void await_suspend(std::coroutine_handle<> h) const {
        context.await(timePoint, [h] { h.resume(); });
    }

    constexpr void await_resume() const noexcept {}
};

}

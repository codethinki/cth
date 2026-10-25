#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/awaiters/base/wait_awaiter_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/chrono.hpp>

#include <coroutine>


namespace cth::co {
/**
 * waits until the time point, resumes the caller on the context
 * @details a failed wait throws in the caller
 */
struct time_point_awaiter : wait_awaiter_base {
    constexpr time_point_awaiter(chrono::time_point_t tp, co::context const& ctx) : timePoint{tp}, context{ctx} {}

    chrono::time_point_t timePoint;
    co::context const& context;

    [[nodiscard]] bool await_ready() const noexcept { return chrono::clock_t::now() >= timePoint; }

    void await_suspend(std::coroutine_handle<> h) { context.await(timePoint, resume(h)); }
};

}

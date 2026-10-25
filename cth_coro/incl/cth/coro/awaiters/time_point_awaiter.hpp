#pragma once
#include "cth/coro/context.hpp"
#include "cth/coro/awaiters/base/wait_awaiter_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/chrono.hpp>

#include <coroutine>


namespace cth::co {
/**
 * waits until the time point, resumes the caller on the context
 * @details awaitable in any coroutine
 * @details this_coro coroutines get it from @ref this_coro::wait() and @ref this_coro::wait_until()
 * @details a failed wait throws in the caller
 */
struct time_point_awaiter : wait_awaiter_base {
    constexpr time_point_awaiter(chrono::time_point_t tp, co::context const& ctx) : timePoint{tp}, context{ctx} {}

    chrono::time_point_t timePoint;
    co::context const& context;

    /**
     * checks if the time point is past
     */
    [[nodiscard]] bool await_ready() const noexcept { return chrono::clock_t::now() >= timePoint; }

    /**
     * resumes the caller on the context once the time point is reached
     * @param h coroutine of the caller
     */
    void await_suspend(std::coroutine_handle<> h) { context.await(timePoint, resume(h)); }
};

}

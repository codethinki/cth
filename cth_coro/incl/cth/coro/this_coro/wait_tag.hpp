#pragma once
#include "cth/coro/awaiters/time_point_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <chrono>


namespace cth::co::this_coro {
/**
 * tag of `wait()` and `wait_until()`
 */
struct [[nodiscard]] wait_tag : tag_base {
    constexpr explicit wait_tag(chrono::time_point_t t) : timePoint{t} {}
    /**
     * waits until the time point on the context of the caller
     * @return awaiter that waits
     */
    constexpr auto operator()(co::context const& c, auto const&) const { return time_point_awaiter{timePoint, c}; }

    chrono::time_point_t timePoint;
};


/**
 * waits until the time point, no wait if it is past
 * @param time_point to wait for
 * @return tag, `co_await` it
 */
constexpr auto wait_until(chrono::time_point_t time_point) { return wait_tag{time_point}; }

/**
 * waits for the duration
 * @details the call reads the clock: the time point is now + `duration`
 * @param duration to wait for
 * @return tag, `co_await` it
 */
template<class Rep, class Period>
auto wait(std::chrono::duration<Rep, Period> const& duration) {
    return wait_tag{chrono::clock_t::now() + duration};
}

}

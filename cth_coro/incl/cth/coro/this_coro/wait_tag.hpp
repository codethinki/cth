#pragma once
#include "cth/coro/awaiters/wait_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <chrono>


namespace cth::co::this_coro {
struct [[nodiscard]] wait_tag : tag_base {
    constexpr explicit wait_tag(chrono::time_point_t t) : timePoint{t} {}
    constexpr auto operator()(co::context const& c, auto const&) const { return wait_awaiter{timePoint, c}; }

    chrono::time_point_t timePoint;
};


/**
 * waits for the time point, noop if in the past
 * @param time_point to wait for
 * @return wait tag
 */
constexpr auto wait_until(chrono::time_point_t time_point) { return wait_tag{time_point}; }

/**
 * waits for a specific duration
 * @param duration to wait for
 * @return wait tag
 */
template<class Rep, class Period>
auto wait(std::chrono::duration<Rep, Period> const& duration) {
    return wait_tag{chrono::clock_t::now() + duration};
}

}

#pragma once
#include "cth/coro/awaiters/wait_awaiter.hpp"
#include "cth/coro/this_coro/payload.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <chrono>


namespace cth::co::this_coro {
struct wait_tag : tag_base {
    constexpr explicit wait_tag(time_point_t t) : timePoint{t} {}
    constexpr auto operator()(payload const& p) const {
        return wait_awaiter{timePoint, p.scheduler()};
    }

    time_point_t timePoint;
};


constexpr auto wait_until(time_point_t time_point) { return wait_tag{time_point}; }

template<class Rep, class Period>
constexpr auto wait(std::chrono::duration<Rep, Period> const& duration) {
    return wait_tag{clock_t::now() + duration};
}

}

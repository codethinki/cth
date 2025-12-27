#pragma once
#include "cth/coro/scheduler.hpp"
#include "cth/coro/this_coro.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <chrono>
#include <coroutine>


namespace cth::co {
struct wait_awaiter {
    std::chrono::steady_clock::time_point timePoint;
    co::scheduler& scheduler;

    constexpr [[nodiscard]] bool await_ready() const noexcept {
        return std::chrono::steady_clock::now() >= timePoint;
    }

    void await_suspend(std::coroutine_handle<> h) const {
        scheduler.await(timePoint, [h]() { h.resume(); });
    }

    void await_resume() const noexcept {}
};

}


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

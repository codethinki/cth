#pragma once
#include "cth/os/exception.hpp"
#include "cth/os/timer.hpp"

#include <algorithm>
#include <cstdint>

#include <sys/timerfd.h>
#include <unistd.h>

/**
 * @file linux/timer.hpp
 * @brief posix implementation of cth/os/timer.hpp, a timerfd on CLOCK_MONOTONIC
 * @attention included by src/timer.cpp only, not a standalone header
 */
namespace cth::os {
namespace {

    timespec to_due_time(timer::time_point_t time_point) {
        using namespace std::chrono;

        auto const now = timer::clock_t::now();
        auto const delta = time_point > now ? time_point - now : timer::clock_t::duration::zero();

        // an all zero it_value disarms instead of firing, so a due-now timer gets the smallest delay
        auto const delay = std::max<nanoseconds>(nanoseconds{1}, duration_cast<nanoseconds>(delta));
        auto const secs = duration_cast<seconds>(delay);

        return timespec{
            .tv_sec = static_cast<std::time_t>(secs.count()),
            .tv_nsec = static_cast<long>((delay - secs).count())
        };
    }

    /**
     * drops a pending expiration so the timerfd stops polling as ready
     */
    void drain(native_handle_t handle) noexcept {
        // non blocking, so this is EAGAIN when nothing expired
        std::uint64_t expirations{};
        [[maybe_unused]] auto const read = ::read(handle, &expirations, sizeof(expirations));
    }

}


timer::timer(bool auto_reset) : _handle{create_timer_handle(auto_reset)}, _autoReset{auto_reset} {}

void timer::set(time_point_t time_point) {
    itimerspec const spec{.it_interval = {}, .it_value = to_due_time(time_point)};

    CTH_OS_STABLE_THROW(::timerfd_settime(native_handle(), 0, &spec, nullptr) != 0, "failed to set timerfd") {}
}

void timer::cancel() {
    // an all zero itimerspec disarms
    itimerspec const spec{};

    CTH_OS_STABLE_THROW(::timerfd_settime(native_handle(), 0, &spec, nullptr) != 0, "failed to cancel timerfd") {}
}

void timer::reset() {
    cancel();
    drain(native_handle());
}

wait_result timer::wait(std::chrono::milliseconds timeout) {
    auto const result = os::wait(native_handle(), timeout);

    // a timerfd keeps polling as ready until its expiration count is taken
    if(_autoReset && result == wait_result::WAITED)
        drain(native_handle());

    return result;
}

unique_native_handle timer::create_timer_handle([[maybe_unused]] bool auto_reset) {
    // auto_reset is not a creation flag here, wait() drains instead
    unique_native_handle handle{::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)};

    CTH_OS_STABLE_THROW(!handle, "failed to create timerfd") {}

    return handle;
}

}

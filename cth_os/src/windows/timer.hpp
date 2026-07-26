#pragma once
#include "windows/win_include.hpp"

#include "cth/os/exception.hpp"
#include "cth/os/timer.hpp"

#include <algorithm>
#include <cstdint>

/**
 * @file windows/timer.hpp
 * @brief windows implementation of cth/os/timer.hpp, a high resolution waitable timer
 * @attention included by src/timer.cpp only, not a standalone header
 */
namespace cth::os {
namespace {

    LARGE_INTEGER to_due_time(timer::time_point_t time_point) {
        LARGE_INTEGER li;

        if(time_point == timer::time_point_t::max()) {
            // largest magnitude negative value, the furthest relative offset expressible
            li.QuadPart = -0x7FFFFFFFFFFFFFFFLL;
            return li;
        }

        auto const now = timer::clock_t::now();
        auto const delta = time_point > now ? time_point - now : timer::clock_t::duration::zero();

        // negative == relative to now, which dodges the wall clock. clamped to -1 because win32
        // documents a due time of 0 oddly
        auto const ticks = std::chrono::duration_cast<std::chrono::nanoseconds>(delta).count() / 100;
        li.QuadPart = -std::max<std::int64_t>(1, ticks);

        return li;
    }

}


timer::timer(bool auto_reset) : _handle{create_timer_handle(auto_reset)}, _autoReset{auto_reset} {}

void timer::set(time_point_t time_point) {
    auto const dueTime = to_due_time(time_point);

    // period 0 fires once, no completion routine because we wait on the handle, no power state resume
    auto const success = SetWaitableTimer(native_handle(), &dueTime, 0, nullptr, nullptr, false);

    CTH_OS_STABLE_THROW(!success, "failed to set waitable timer") {}
}

void timer::cancel() {
    CTH_OS_STABLE_THROW(!CancelWaitableTimer(native_handle()), "failed to cancel timer") {}
}

void timer::reset() {
    // arming clears the signaled state, so set-then-cancel is how a signaled timer is cleared
    set(time_point_t::max());
    cancel();
}

wait_result timer::wait(std::chrono::milliseconds timeout) {
    // the kernel consumes the signal for an auto reset timer, nothing to do here
    return os::wait(native_handle(), timeout);
}

unique_native_handle timer::create_timer_handle(bool auto_reset) {
    DWORD flags = CREATE_WAITABLE_TIMER_HIGH_RESOLUTION;

    if(!auto_reset)
        flags |= CREATE_WAITABLE_TIMER_MANUAL_RESET;

    unique_native_handle handle{CreateWaitableTimerExW(nullptr, nullptr, flags, TIMER_ALL_ACCESS)};

    CTH_OS_STABLE_THROW(!handle, "failed to create timer handle") {}

    return handle;
}

}

#include "cth/win/coro/timer.hpp"

#include "../win_include.hpp"


namespace cth::win::co {

namespace {
    LARGE_INTEGER time_point_to_large_integer(std::chrono::steady_clock::time_point time_point) {
        using namespace std::chrono;


        LARGE_INTEGER li;

        if(time_point == steady_clock::time_point::max()) {
            // -0x7FFFFFFFFFFFFFFFLL is the largest magnitude negative number
            li.QuadPart = -0x7FFFFFFFFFFFFFFFLL;
            return li;
        }
        auto const now = steady_clock::now();

        // If the time is in the past, set to -1 (fire immediately)
        if(time_point <= now) {
            li.QuadPart = -1;
            return li;
        }

        // Calculate duration from now
        auto const duration = time_point - now;

        // Convert to 100-nanosecond intervals (ticks)
        int64_t const ticks = duration_cast<nanoseconds>(duration).count() / 100;

        // Windows API: Negative value means "Relative offset from now"
        // We ensure it is at least -1 to avoid 0 (which sometimes behaves oddly in legacy docs)
        li.QuadPart = -std::max<int64_t>(1, ticks);

        return li;
    }
}


timer::timer(bool auto_reset) : _handle{create_timer_handle(auto_reset)} {}

void timer::set(time_point_t time_point) {
    auto const liDueTime = time_point_to_large_integer(time_point);

    // 2nd arg: DueTime (Negative = relative to now)
    // 3rd arg: Period (0 = signal once, then stop. We don't want a periodic loop)
    // 4th arg: CompletionRoutine (NULL because we are likely waiting on the handle or using IOCP)
    // 5th arg: ArgToCompletion (NULL)
    // 6th arg: Resume (FALSE - don't wake system from suspended power state)
    auto const success = SetWaitableTimer(native_handle(), &liDueTime, 0, nullptr, nullptr, false);

    CTH_WIN_STABLE_THROW(!success, "failed to set waitable timer") {}
}
void timer::cancel() {
    auto const success = CancelWaitableTimer(native_handle());

    CTH_WIN_STABLE_THROW(!success, "failed to cancel timer") {}
}
void timer::reset() {
    set(time_point_t::max());
    cancel();
}

closing_handle timer::create_timer_handle(bool auto_reset) {
    DWORD flags = CREATE_WAITABLE_TIMER_HIGH_RESOLUTION;

    if(!auto_reset)
        flags |= CREATE_WAITABLE_TIMER_MANUAL_RESET;


    closing_handle handle{CreateWaitableTimerExW(nullptr, nullptr, flags, TIMER_ALL_ACCESS)};

    CTH_WIN_STABLE_THROW(handle == nullptr, "failed to create timer handle") {}

    return handle;
}
}

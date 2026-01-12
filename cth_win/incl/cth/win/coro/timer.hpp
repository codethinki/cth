#pragma once
#include "wait.hpp"

#include "cth/hash/hash_general.hpp"
#include "cth/win/win_types.hpp"


namespace cth::win::co {
/**
 * RAII wrapper for windows timer handles
 * @note requires >= WIN10 1803
 */
class timer {
public:
    using clock_t = std::chrono::steady_clock;
    using time_point_t = clock_t::time_point;

    /**
     * @param auto_reset if true, timer automatically resets after being signaled & waited on
     * @throws cth::except::win_exception if creation fails
     */
    timer(bool auto_reset = true);
    ~timer() = default;

    /**
     * sets the timer to fire at give time point, overwrites previous points
     * @throws cth::except::win_exception if setting the timer fails
     */
    void set(time_point_t);

    /**
     * cancels the timer if active
     * @details if a timer is already signaled, cancel has no effect
     * @throws cth::except::win_exception if cancelling the timer fails
     */
    void cancel();

    /**
     * resets the signaled state and cancels the timer
     * @details calls @ref cancel()
     * @throws cth::except::win_exception if signal state reset fails
     * @throws cth::except::win_exception if @ref cancel() fails
     */
    void reset();

    /**
     * blocks until timer or ms time out
     * @param ms to timeout on
     * @details calls @ref cth::win::co::wait(void*, unsigned long)
     */
    WaitResult wait(unsigned long ms) {
        return co::wait(native_handle(), ms);
    }

private:
    /**
     * @throws cth::except::win_exception if creation fails
     */
    static closing_handle create_timer_handle(bool auto_reset);

    closing_handle _handle;

public:
    closing_handle::pointer native_handle() const noexcept { return _handle.get(); }
};

constexpr size_t hash(timer const& t) {
    return cth::hash::combine(t.native_handle());
}

}



CTH_HASH_OVERLOAD(cth::win::co::timer, cth::win::co::hash);

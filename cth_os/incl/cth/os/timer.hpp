#pragma once
#include "cth/os/native_handle.hpp"
#include "cth/os/wait.hpp"

#include <cth/hash/hash_general.hpp>

#include <chrono>


namespace cth::os {

/**
 * RAII wrapper for os timer handles
 * @note windows requires >= WIN10 1803
 */
class timer {
public:
    using clock_t = std::chrono::steady_clock;
    using time_point_t = clock_t::time_point;

    /**
     * @param auto_reset if true, timer automatically resets after being signaled & waited on
     * @throws cth::except::os_exception if creation fails
     */
    explicit timer(bool auto_reset = true);
    ~timer() = default;

    /**
     * sets the timer to fire at give time point, overwrites previous points
     * @throws cth::except::os_exception if setting the timer fails
     */
    void set(time_point_t);

    /**
     * cancels the timer if active
     * @details if a timer is already signaled, cancel has no effect
     * @throws cth::except::os_exception if cancelling the timer fails
     */
    void cancel();

    /**
     * resets the signaled state and cancels the timer
     * @details calls @ref cancel()
     * @throws cth::except::os_exception if signal state reset fails
     * @throws cth::except::os_exception if @ref cancel() fails
     */
    void reset();

    /**
     * blocks until timer or timeout
     * @param timeout to time out on
     * @details calls @ref cth::os::wait(native_handle_t, std::chrono::milliseconds)
     * @details consumes the signal if @ref auto_reset()
     */
    wait_result wait(std::chrono::milliseconds timeout);

private:
    /**
     * @throws cth::except::os_exception if creation fails
     */
    static unique_native_handle create_timer_handle(bool auto_reset);

    unique_native_handle _handle;
    bool _autoReset;

public:
    [[nodiscard]] native_handle_t native_handle() const noexcept { return _handle.get(); }

    [[nodiscard]] bool auto_reset() const noexcept { return _autoReset; }

    timer(timer const&) = delete;
    timer& operator=(timer const&) = delete;
    timer(timer&&) noexcept = default;
    timer& operator=(timer&&) noexcept = default;
};

constexpr size_t hash(timer const& t) { return cth::hash::combine(t.native_handle()); }

}


CTH_HASH_OVERLOAD(cth::os::timer, cth::os::hash);

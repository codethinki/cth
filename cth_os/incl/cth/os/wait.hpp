#pragma once
#include "cth/os/native_handle.hpp"

#include <chrono>


namespace cth::os {

enum class wait_result {
    WAITED,
    TIMEOUT,
    /**
     * @note windows only
     */
    ABANDONED
};

/**
 * timeout for @ref wait() that never elapses
 */
inline constexpr auto INFINITE_WAIT = std::chrono::milliseconds::max();

/**
 * waits for the handle
 * @param handle to wait on
 * @param timeout max wait, 0: no wait just state query, @ref INFINITE_WAIT: infinite wait
 * @throws cth::except::os_exception if the wait fails
 */
wait_result wait(native_handle_t handle, std::chrono::milliseconds timeout);

/**
 * queries the handles signaled state, never blocks
 * @return true if signaled
 * @attention consumes the signal of an auto reset handle
 * @throws cth::except::os_exception if the query fails
 */
[[nodiscard]] bool check_ready(native_handle_t handle);

}

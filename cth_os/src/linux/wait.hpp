#pragma once
#include "cth/os/exception.hpp"
#include "cth/os/wait.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

#include <poll.h>

/**
 * @file linux/wait.hpp
 * @brief posix implementation of cth/os/wait.hpp
 * @attention included by src/wait.cpp only, not a standalone header
 */
namespace cth::os {
namespace {

    int to_native_timeout(std::chrono::milliseconds timeout) {
        if(timeout >= INFINITE_WAIT)
            return -1;

        if(timeout.count() <= 0)
            return 0;

        return static_cast<int>(std::min<std::int64_t>(timeout.count(), std::numeric_limits<int>::max()));
    }

}


wait_result wait(native_handle_t handle, std::chrono::milliseconds timeout) {
    pollfd pfd{.fd = handle, .events = POLLIN, .revents = 0};

    auto const result = ::poll(&pfd, 1, to_native_timeout(timeout));

    CTH_OS_STABLE_THROW(result < 0, "poll failed") {}

    // poll has no abandoned equivalent
    return result > 0 ? wait_result::WAITED : wait_result::TIMEOUT;
}

bool check_ready(native_handle_t handle) {
    return wait(handle, std::chrono::milliseconds{0}) == wait_result::WAITED;
}

}

#pragma once
#include "windows/win_include.hpp"

#include "cth/os/exception.hpp"
#include "cth/os/wait.hpp"

#include <algorithm>
#include <cstdint>
#include <utility>

/**
 * @file windows/wait.hpp
 * @brief windows implementation of cth/os/wait.hpp
 * @attention included by src/wait.cpp only, not a standalone header
 */
namespace cth::os {
namespace {

    DWORD to_native_timeout(std::chrono::milliseconds timeout) {
        if(timeout >= INFINITE_WAIT)
            return INFINITE;

        if(timeout.count() <= 0)
            return 0;

        return static_cast<DWORD>(std::min<std::int64_t>(timeout.count(), INFINITE - 1));
    }

    wait_result convert(DWORD result) {
        switch(result) {
            case WAIT_OBJECT_0: return wait_result::WAITED;
            case WAIT_TIMEOUT: return wait_result::TIMEOUT;
            case WAIT_ABANDONED: return wait_result::ABANDONED;
            case WAIT_FAILED: throw cth::except::os_exception{"wait failed"};
            default: CTH_CRITICAL(false, "illegal wait result: {}", result) {
                }
        }
        std::unreachable();
    }

}


wait_result wait(native_handle_t handle, std::chrono::milliseconds timeout) {
    return convert(WaitForSingleObject(handle, to_native_timeout(timeout)));
}

bool check_ready(native_handle_t handle) {
    return wait(handle, std::chrono::milliseconds{0}) == wait_result::WAITED;
}

}

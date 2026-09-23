#pragma once

#include "windows/win_include.hpp"

#include "cth/os/exception.hpp"
#include "cth/os/fence.hpp"

/**
 * @file windows/fence.hpp
 * @brief Windows implementation of cth/os/fence.hpp
 * @attention included by src/fence.cpp only, not a standalone header
 */
namespace cth::os {

void fence::signal() noexcept {
    CTH_OS_STABLE_ERR(!SetEvent(native_handle()), "failed to signal fence") {}
}

void fence::reset() noexcept {
    CTH_OS_STABLE_ERR(!ResetEvent(native_handle()), "failed to reset fence") {}
}

unique_native_handle fence::create_handle(bool signaled) {
    unique_native_handle handle{CreateEventW(nullptr, true, signaled, nullptr)};
    CTH_OS_STABLE_THROW(!handle, "failed to create fence") {}
    return handle;
}

}

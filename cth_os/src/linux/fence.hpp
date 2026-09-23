#pragma once

#include "cth/os/exception.hpp"
#include "cth/os/fence.hpp"

#include <cerrno>
#include <cstdint>

#include <sys/eventfd.h>
#include <unistd.h>

/**
 * @file linux/fence.hpp
 * @brief Linux implementation of cth/os/fence.hpp
 * @attention included by src/fence.cpp only, not a standalone header
 */
namespace cth::os {

void fence::signal() noexcept {
    std::uint64_t value = 1;
    auto const written = ::write(native_handle(), &value, sizeof(value));
    CTH_OS_STABLE_ERR(written != sizeof(value) && errno != EAGAIN, "failed to signal fence") {}
}

void fence::reset() noexcept {
    std::uint64_t value{};
    while(::read(native_handle(), &value, sizeof(value)) == sizeof(value)) {}
    CTH_OS_STABLE_ERR(errno != EAGAIN, "failed to reset fence") {}
}

unique_native_handle fence::create_handle(bool signaled) {
    unique_native_handle handle{
        ::eventfd(signaled ? 1u : 0u, EFD_CLOEXEC | EFD_NONBLOCK)
    };
    CTH_OS_STABLE_THROW(!handle, "failed to create fence") {}
    return handle;
}

}

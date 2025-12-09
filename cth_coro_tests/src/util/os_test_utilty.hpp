#pragma once
#include <cth/os/osdef.hpp>
#include <cth/coro/os/native_handle.hpp>

#ifdef CTH_FS_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#elifdef CTH_FS_POSIX
#include <sys/eventfd.h>
#include <unistd.h>
#endif

namespace cth::co::os {

inline native_handle_t create_event() {
#ifdef CTH_FS_WINDOWS
    return CreateEventW(nullptr, TRUE, FALSE, nullptr);
#elifdef CTH_FS_POSIX
    return eventfd(0, EFD_NONBLOCK);
#endif
}

inline void signal_event(native_handle_t handle) {
#ifdef CTH_FS_WINDOWS
    SetEvent(handle);
#elifdef CTH_FS_POSIX
    uint64_t val = 1;
    [[maybe_unused]] auto result = write(handle, &val, sizeof(val));
#endif
}

inline void close_handle(native_handle_t handle) {
#ifdef CTH_FS_WINDOWS
    CloseHandle(handle);
#elifdef CTH_FS_POSIX
    close(handle);
#endif
}

} // namespace cth::co::os
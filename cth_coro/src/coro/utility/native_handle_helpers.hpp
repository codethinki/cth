#pragma once
#include "cth/coro/os/native_handle.hpp"

#ifdef CTH_FS_WINDOWS
#include <boost/asio/windows/object_handle.hpp>
#elifdef CTH_FS_LINUX
#include <boost/asio/posix/stream_descriptor.hpp>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <memory>

namespace bas = boost::asio;

namespace cth::co {
using native_handle_handler_t =
#ifdef CTH_FS_WINDOWS
    bas::windows::object_handle;
#elifdef CTH_FS_LINUX
    bas::posix::stream_descriptor;
#endif

}

namespace cth::co {
/**
 * duplicates the native handle on windows and linux
 * @param handle to duplicate
 * @return duplicated handle
 * @throws cth::except::default_exception if duplication fails
 */
inline os::native_handle_t duplicate_awaitable_native_handle(os::native_handle_t handle) {
#ifdef CTH_FS_WINDOWS
    HANDLE dup = nullptr;

    auto const success = ::DuplicateHandle(
        ::GetCurrentProcess(),
        static_cast<HANDLE>(handle),
        ::GetCurrentProcess(),
        &dup,
        // Target Handle Pointer
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS // Options
    );

    CTH_STABLE_THROW(!success, "failed to duplicate native win object handle {}", handle)
    details->add("last error: {}", ::GetLastError());

    return dup;

#elifdef CTH_FS_POSIX

    int dupFd = ::fcntl(handle, F_DUPFD_CLOEXEC, 0);

    CTH_STABLE_THROW(dupFd == -1, "failed to duplicate native posix handle: {}", handle) {}

    return dupFd;
#endif
}

inline std::unique_ptr<native_handle_handler_t>
wrap_unique(os::native_handle_t handle, bas::io_context& ctx) {
    return std::make_unique<native_handle_handler_t>(ctx, duplicate_awaitable_native_handle(handle));
}
}

#pragma once
#include <cth/os/native_handle.hpp>
#include <cth/os/osdef.hpp>

#ifdef CTH_FS_WINDOWS
#include <asio/windows/object_handle.hpp>
#elifdef CTH_FS_POSIX
#include <asio/posix/stream_descriptor.hpp>
#endif

#include <memory>
#include <utility>

namespace bas = asio;

namespace cth::co {
using native_handle_handler_t =
#ifdef CTH_FS_WINDOWS
    bas::windows::object_handle;
#elifdef CTH_FS_POSIX
    bas::posix::stream_descriptor;
#endif

}

namespace cth::co {

/**
 * wraps a duplicate of the handle in a boost handler
 * @details the handler closes what it is given, so it gets its own duplicate and the caller keeps theirs
 * @throws cth::except::os_exception if the duplication fails
 */
inline std::unique_ptr<native_handle_handler_t>
wrap_unique(os::native_handle_t handle, bas::io_context& ctx) {
    return std::make_unique<native_handle_handler_t>(ctx, os::duplicate_handle(handle));
}

/**
 * asynchronously waits until the handle is signaled
 * @details posix descriptors (eventfd, timerfd) signal by becoming readable and need the wait type,
 * windows object handles have no wait type
 */
template<class Handle, class Handler>
void async_wait_signaled(Handle& handle, Handler&& completion) {
    if constexpr(requires { Handle::wait_read; })
        handle.async_wait(Handle::wait_read, std::forward<Handler>(completion));
    else
        handle.async_wait(std::forward<Handler>(completion));
}

}

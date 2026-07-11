#include "cth/coro/os/native_handle.hpp"

#include <cth/os/osdef.hpp>

#if !defined(CTH_FS_POSIX) && !defined(CTH_FS_WINDOWS)
#error "fuck"
#endif


#ifdef CTH_FS_WINDOWS
#include <cth/win/coro/wait.hpp>
#include <cth/win/win_types.hpp>
#elifdef CTH_FS_POSIX
#include <poll.h>
#endif


namespace cth::co::os {


bool check_ready(os::native_handle_t handle) {
#ifdef CTH_FS_WINDOWS
    try {
        return cth::win::co::wait(handle, 0) == win::co::WaitResult::WAITED;
    } catch(cth::except::win_exception const& e) {
        throw except::coro_exception{e.msg(), e.severity(), e.location(), e.stacktrace()};
    }
#elifdef CTH_FS_POSIX
    pollfd pfd{fd, POLLIN, 0};
    return poll(&pfd, 1, 0) > 0;
#endif
}
}

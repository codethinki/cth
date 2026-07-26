#include "cth/os/native_handle.hpp"

#include "cth/os/exception.hpp"

#ifdef CTH_FS_WINDOWS
#include "windows/win_include.hpp"
#elifdef CTH_FS_POSIX
#include <fcntl.h>
#include <unistd.h>
#endif


namespace cth::os {

bool close_handle(native_handle_t handle) noexcept {
    if(handle == invalid_handle())
        return false;

#ifdef CTH_FS_WINDOWS
    return CloseHandle(handle) != 0;
#elifdef CTH_FS_POSIX
    return ::close(handle) == 0;
#endif
}

native_handle_t duplicate_handle(native_handle_t handle) {
#ifdef CTH_FS_WINDOWS
    HANDLE duplicate = nullptr;

    auto const success = ::DuplicateHandle(
        ::GetCurrentProcess(),
        handle,
        ::GetCurrentProcess(),
        &duplicate,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS
    );

    CTH_OS_STABLE_THROW(!success, "failed to duplicate handle [{}]", handle) {
        details->add("last error: {}", ::GetLastError());
    }

    return duplicate;
#elifdef CTH_FS_POSIX
    auto const duplicate = ::fcntl(handle, F_DUPFD_CLOEXEC, 0);

    CTH_OS_STABLE_THROW(duplicate == invalid_handle(), "failed to duplicate handle [{}]", handle) {}

    return duplicate;
#endif
}

}

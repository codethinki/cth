#pragma once
#include <cth/coro/exception.hpp>
#include <cth/os/osdef.hpp>

#ifdef CTH_FS_WINDOWS
#define CTH_CO_WIN_POSIX_VALUE(win, posix) win
#elifdef CTH_FS_POSIX
#define CTH_CO_WIN_POSIX_VALUE(win, posix) posix
#else
#error "unsupported filesystem used"
#endif


namespace cth::co::os {

using native_handle_t = CTH_CO_WIN_POSIX_VALUE(void*, int);


/**
 * checks if the native handle is ready
 * @throws cth::except::coro_exception if the check fails
 */
bool check_ready(os::native_handle_t);

}

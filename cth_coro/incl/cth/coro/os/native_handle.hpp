#pragma once
#include <cth/os/osdef.hpp>

namespace cth::co::os {
using native_handle_t =
#ifdef CTH_FS_WINDOWS
void*;
#elifdef CTH_FS_POSIX
int;
#else
#error "unsupported cth_coro file system
#endif


}

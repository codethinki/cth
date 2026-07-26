#include <cth/os/osdef.hpp>

#ifdef CTH_FS_WINDOWS
#include "windows/timer.hpp"
#elifdef CTH_FS_POSIX
#include "linux/timer.hpp"
#endif

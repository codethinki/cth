#include <cth/os/osdef.hpp>

#ifdef CTH_FS_WINDOWS
#include "windows/wait.hpp"
#elifdef CTH_FS_POSIX
#include "linux/wait.hpp"
#endif

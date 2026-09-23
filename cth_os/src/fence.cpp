#include <cth/os/osdef.hpp>

#include "cth/os/fence.hpp"

#ifdef CTH_FS_WINDOWS
#include "windows/fence.hpp"
#elifdef CTH_FS_POSIX
#include "linux/fence.hpp"
#endif

namespace cth::os {

fence::fence(bool signaled) : _handle{create_handle(signaled)} {}

wait_result fence::wait(std::chrono::milliseconds timeout) const {
    return os::wait(native_handle(), timeout);
}

bool fence::signaled() const {
    return os::check_ready(native_handle());
}

}

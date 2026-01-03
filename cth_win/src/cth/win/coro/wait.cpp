#include "cth/win/coro/wait.hpp"

#include "../win_include.hpp"


namespace cth::win::co {

namespace {
    WaitResult check_convert(DWORD result) {
        switch(result) {
            case WAIT_OBJECT_0:
                return WaitResult::WAITED;
            case WAIT_TIMEOUT:
                return WaitResult::TIMEOUT;
            case WAIT_ABANDONED:
                return WaitResult::ABANDONED;
            case WAIT_FAILED:
                throw cth::except::win_exception{"wait failed"};
            default:
                CTH_CRITICAL(false, "illegal wait result: {}", result){}
        }
        std::unreachable();
    }
}


WaitResult wait(void* native_handle, unsigned long ms) {
    return check_convert(WaitForSingleObject(native_handle, ms));
}
}

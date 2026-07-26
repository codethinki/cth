#include "cth/win/coro/wait.hpp"

#include "../win_include.hpp"


namespace cth::win::co {

namespace {
    wait_result check_convert(DWORD result) {
        switch(result) {
            case WAIT_OBJECT_0: return wait_result::WAITED;
            case WAIT_TIMEOUT: return wait_result::TIMEOUT;
            case WAIT_ABANDONED: return wait_result::ABANDONED;
            case WAIT_FAILED: throw cth::except::win_exception{"wait failed"};
            default: CTH_CRITICAL(false, "illegal wait result: {}", result) {
                }
        }
        std::unreachable();
    }
}


wait_result wait(void* native_handle, unsigned long ms) {
    return check_convert(WaitForSingleObject(native_handle, ms));
}
}

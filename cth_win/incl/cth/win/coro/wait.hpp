#pragma once

namespace cth::win::co {
enum class wait_result { WAITED, TIMEOUT, ABANDONED };

/**
 * waits for the handle
 * @param native_handle to wait on
 * @param ms to wait max for,  0: no wait just state query, MAX: infinite wait
 */
wait_result wait(void* native_handle, unsigned long ms);


}

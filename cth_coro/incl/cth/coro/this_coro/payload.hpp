#pragma once
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::this_coro {
/**
 * base of custom payloads, carries no data
 * @details @ref default_payload, a custom payload slices to it when its task awaits a task with the default payload
 */
struct payload_base {};

}

#pragma once
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::dev {
/**
 * promise part of tasks that continue the parent on its context
 * @details @ref affine_final_promise_awaiter reads it
 */
struct affine_promise_base {
    /**
     * context the parent continues on, the awaiter of the task sets it before the task runs
     */
    co::context const* parentContext = nullptr;
};
}

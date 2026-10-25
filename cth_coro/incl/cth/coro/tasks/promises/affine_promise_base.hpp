#pragma once
#include "cth/coro/utility/fwd.hpp"


namespace cth::co::dev {
/**
 * promise part of tasks that continue the parent on its scheduler
 * @details @ref affine_final_promise_awaiter reads it
 */
struct affine_promise_base {
    /**
     * scheduler the parent continues on, the awaiter of the task sets it before the task runs
     */
    co::scheduler const* parentScheduler = nullptr;
};
}

#pragma once
#include "basic_promise.hpp"

#include "cth/coro/tasks/awaiters/dev/affine_final_promise_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co::dev {
/**
 * promise of tasks that continue the parent on its context
 * @details its final awaiter is @ref affine_final_promise_awaiter, it reads @ref parentContext
 */
template<class T>
struct affine_promise : basic_promise<T, std::suspend_always{}, affine_final_promise_awaiter{}> {
    /**
     * context the parent continues on, the awaiter of the task sets it before the task runs
     */
    co::context const* parentContext = nullptr;
};
}

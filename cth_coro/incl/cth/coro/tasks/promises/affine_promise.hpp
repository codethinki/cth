#pragma once
#include "basic_promise.hpp"

#include "cth/coro/tasks/awaiters/dev/affine_final_promise_awaiter.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <coroutine>


namespace cth::co::dev {
/**
 * promise of tasks that continue the caller on its context
 * @details its final awaiter is @ref affine_final_promise_awaiter, it reads @ref callerContext
 * @tparam T result type
 */
template<class T>
struct affine_promise : basic_promise<T, std::suspend_always{}, affine_final_promise_awaiter{}> {
    /**
     * stores the context of the caller, the task continues the caller on it
     * @param caller promise of the caller
     */
    template<class CallerPromise>
    constexpr void attach(CallerPromise const& caller) noexcept {
        static_assert(this_coro_compatible<CallerPromise>, "the caller must be a this_coro coroutine");
        callerContext = &caller.context();
    }

    /**
     * context the caller continues on, @ref attach() sets it before the task runs
     */
    co::context const* callerContext = nullptr;
};
}

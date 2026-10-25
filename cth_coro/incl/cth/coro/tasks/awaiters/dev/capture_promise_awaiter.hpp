#pragma once
#include "basic_promise_awaiter.hpp"

#include "cth/coro/utility/fwd.hpp"

#include <coroutine>

namespace cth::co::dev {

/**
 * awaiter of @ref capture_task, the capture continues the caller on its scheduler
 */
template<class Promise>
struct capture_promise_awaiter
    : this_coro_awaiter_base
    , basic_promise_awaiter<Promise> {
    using dev::basic_promise_awaiter<Promise>::basic_promise_awaiter;

    /**
     * stores the scheduler of the caller and starts the capture
     * @param caller this_coro coroutine that awaits the capture
     * @return handle of the capture
     */
    template<class Caller>
    auto await_suspend(std::coroutine_handle<Caller> caller) noexcept -> std::coroutine_handle<Promise> {
        static_assert(this_coro_compatible<Caller>, "captures need a this_coro caller");

        auto& capture = this->handle.promise();
        capture.parentScheduler = &caller.promise().scheduler();
        capture.continuation = caller;
        return this->handle;
    }
};
}

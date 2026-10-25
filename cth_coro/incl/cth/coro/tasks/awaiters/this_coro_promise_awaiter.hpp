#pragma once
#include "cth/coro/tasks/awaiters/dev/basic_promise_awaiter.hpp"
#include "cth/coro/utility/concepts.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <coroutine>

namespace cth::co {
/**
 * awaiter of the tasks of this library, the promise of the task attaches to the caller
 * @details the task continues the caller on its context
 */
template<cth_promise Promise>
struct this_coro_promise_awaiter
    : this_coro_awaiter_base,
    private dev::basic_promise_awaiter<Promise> {
private:
    using base_t = dev::basic_promise_awaiter<Promise>;

public:
    this_coro_promise_awaiter() = delete;

    using promise_type = base_t::promise_type;


    using base_t::await_ready;
    using base_t::await_resume;
    using base_t::base_t;


    /**
     * attaches the task to the caller and starts it
     * @details a throwing attach (a payload copy) resumes the caller with the exception, the task does not start
     * @param caller coroutine that awaits the task
     * @return handle of the task
     */
    template<class Caller>
    auto await_suspend(std::coroutine_handle<Caller> caller) noexcept(dev::nothrow_attachable<promise_type, Caller>)
        -> std::coroutine_handle<promise_type> {
        base_t::handle.promise().attach(caller.promise());
        return base_t::await_suspend(caller);
    }
};
}

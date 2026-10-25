#pragma once
#include "cth/coro/tasks/awaiters/dev/basic_promise_awaiter.hpp"
#include "cth/coro/this_coro/this_coro_base.hpp"
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/concepts.hpp>
#include <cth/meta/coro.hpp>

#include <coroutine>

namespace cth::co {
/**
 * awaiter of this_coro tasks, the task runs with the context and the payload of the caller
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
     * passes the context and the payload of the caller to the task and starts it
     * @details a throwing payload copy resumes the caller with the exception, the task does not start
     * @param caller this_coro coroutine that awaits the task
     * @return handle of the task
     */
    template<class Caller>
    auto await_suspend(std::coroutine_handle<Caller> caller) noexcept(NOTHROW_INHERIT<Caller>)
        -> std::coroutine_handle<promise_type> {
        static_assert(
            this_coro_compatible<Caller>,
            "this_coro tasks need a this_coro caller, use spawn() or sync_wait()"
        );

        auto& child = base_t::handle.promise();
        child.inherit(caller.promise());
        child.parentContext = &caller.promise().context();
        child.continuation = caller;
        return base_t::handle;
    }

private:
    template<class Caller>
    static constexpr bool NOTHROW_INHERIT = [] {
        if constexpr(this_coro_compatible<Caller>)
            return mta::nothrow_constructible<payload_t<promise_type>, payload_t<Caller> const&>;
        else
            return true; // the static_assert in await_suspend reports the caller
    }();
};
}

#pragma once
#include "basic_promise_awaiter.hpp"

#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <coroutine>


namespace cth::co::dev {
/**
 * awaiter of @ref spawn_task, the spawned task keeps its own context and payload
 * @details the spawned task continues the caller on its context,
 *  a caller without a context continues on the context of the executor
 */
template<cth_promise Promise>
struct spawn_promise_awaiter : this_coro_awaiter_base, private basic_promise_awaiter<Promise> {
private:
    using base_t = basic_promise_awaiter<Promise>;

public:
    using promise_type = base_t::promise_type;

    using base_t::await_ready;
    using base_t::await_resume;
    using base_t::base_t;

    /**
     * stores the context the caller continues on and starts the spawned task
     * @param caller coroutine that awaits the spawned task
     * @return handle of the spawned task
     */
    template<class Caller>
    auto await_suspend(std::coroutine_handle<Caller> caller) noexcept -> std::coroutine_handle<promise_type> {
        auto& child = base_t::handle.promise();
        if constexpr(this_coro_compatible<Caller>)
            child.parentContext = &caller.promise().context();
        else
            child.parentContext = &child.context();

        child.continuation = caller;
        return base_t::handle;
    }
};
}

#pragma once
#include "macro.hpp"

#include "../native_handle_helpers.hpp"
#include "../timer_pool.hpp"

#include "cth/coro/context.hpp"

#include <asio/bind_executor.hpp>
#include <asio/dispatch.hpp>
#include <asio/io_context.hpp>
#include <asio/post.hpp>

#include <utility>


namespace cth::co::dev {
/**
 * runs the work on the executor
 * @param executor to run on
 * @param work to run
 */
template<class Executor, class Work>
void post_on(Executor const& executor, Work&& work) {
    bas::post(executor, std::forward<Work>(work));
}

/**
 * calls the callback on the executor once the handle is signaled
 * @param executor to call the callback on
 * @param ctx io_context of the executor, runs the wait
 * @param cb to call
 * @throws cth::except::os_exception if the handle duplication fails
 */
template<class Executor, class Callback>
void await_on(Executor const& executor, bas::io_context& ctx, context::native_handle handle, Callback&& cb) {
    auto handler = wrap_unique(handle, ctx);
    auto& waiter = *handler; // before the move below, argument evaluation order is unspecified
    async_wait_signaled(
        waiter,
        bas::bind_executor(
            executor,
            [
                h = std::move(handler),
                cb = std::forward<Callback>(cb)
            ](bas::error_code const& ec) mutable {
                ASIO_EC_STABLE_THROW(ec, "async wait for handle [{}] failed", h->native_handle())

                cb();
            }
        )
    );
}

/**
 * calls the callback on the executor once the time point is reached
 * @details the timer callback dispatches the callback onto the executor
 * @param executor to call the callback on
 * @param pool timer pool of the io_context of the executor
 * @param time_point to wait for
 * @param cb to call
 */
template<class Executor, class Callback>
void await_on(Executor const& executor, timer_pool& pool, chrono::time_point_t time_point, Callback&& cb) {
    pool.set(
        time_point,
        [executor, cb = std::forward<Callback>(cb)]() mutable { bas::dispatch(executor, std::move(cb)); }
    );
}
}

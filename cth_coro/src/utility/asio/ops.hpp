#pragma once
#include "../native_handle_helpers.hpp"
#include "../timer_pool.hpp"

#include "cth/coro/context.hpp"

#include <asio/bind_executor.hpp>
#include <asio/dispatch.hpp>
#include <asio/io_context.hpp>
#include <asio/post.hpp>

#include <system_error>
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
 * @details the callback gets the error code of the wait
 * @param executor to call the callback on
 * @param ctx io_context of the executor, runs the wait
 * @param cb to call with the error code
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
                h = std::move(handler), // keeps the descriptor alive until the completion
                cb = std::forward<Callback>(cb)
            ](std::error_code const& ec) mutable { cb(ec); }
        )
    );
}

/**
 * calls the callback on the executor once the time point is reached
 * @details the timer callback dispatches the callback onto the executor, the callback gets the error code of the wait
 * @param executor to call the callback on
 * @param pool timer pool of the io_context of the executor
 * @param time_point to wait for
 * @param cb to call with the error code
 */
template<class Executor, class Callback>
void await_on(Executor const& executor, timer_pool& pool, chrono::time_point_t time_point, Callback&& cb) {
    pool.set(
        time_point,
        [executor, cb = std::forward<Callback>(cb)](std::error_code const& ec) mutable {
            bas::dispatch(executor, [cb = std::move(cb), ec]() mutable { cb(ec); });
        }
    );
}
}

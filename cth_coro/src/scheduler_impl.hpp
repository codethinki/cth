#pragma once
#include "cth/coro/scheduler.hpp"

#include "utility/asio.hpp"
#include "utility/native_handle_helpers.hpp"
#include "utility/timer_pool.hpp"


#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>
#include <asio/post.hpp>

#include <optional>


namespace cth::co {

struct scheduler::Impl {
    using guard_t = asio::executor_work_guard<asio::io_context::executor_type>;

    // ReSharper disable once CppNonExplicitConvertingConstructor
    Impl(size_t workers) : ctx{static_cast<int>(workers)}, timerPool{ctx} { ctx.stop(); }

    [[nodiscard]] bool active() const { return workGuard != std::nullopt || !ctx.stopped(); }

    void start() {
        ctx.restart();
        workGuard.emplace(asio::make_work_guard(ctx));
    }
    void request_stop() {
        workGuard = std::nullopt;
        ctx.stop();
    }

    void post(void_func work) { bas::post(ctx, std::move(work)); }

    void await(native_handle handle, void_func callback) {
        auto handler = wrap_unique(handle, ctx);
        auto& waiter = *handler; // before the move below, argument evaluation order is unspecified
        async_wait_signaled(
            waiter,
            [
                h = std::move(handler),
                cb = std::move(callback)
            ](asio::error_code const& ec) mutable {
                ASIO_EC_STABLE_THROW(ec, "async wait for handle [{}] failed", h->native_handle())

                cb();
            }
        );
    }

    void await(chrono::time_point_t time_point, void_func callback) {
        timerPool.set(time_point, std::move(callback));
    }


    bas::io_context ctx;
    std::optional<guard_t> workGuard;

    timer_pool timerPool;
};
}

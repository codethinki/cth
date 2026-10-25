#pragma once
#include "cth/coro/scheduler.hpp"

#include "utility/asio/ops.hpp"
#include "utility/timer_pool.hpp"


#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>

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


    bas::io_context ctx;
    std::optional<guard_t> workGuard;

    timer_pool timerPool;
};
}

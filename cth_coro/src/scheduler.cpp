#include "cth/coro/scheduler.hpp"

#include "utility/asio.hpp"
#include "utility/native_handle_helpers.hpp"
#include "utility/timer_pool.hpp"


#include <cth/numeric.hpp>


#include <asio/io_context.hpp>
#include <asio/post.hpp>


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
        handler->async_wait(
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


namespace cth::co {

scheduler::scheduler(size_t workers) : _impl{std::make_unique<Impl>(workers)},
    _activeWorkers{std::make_unique<std::atomic<size_t>>()},
    _workers{workers} {
    CTH_CRITICAL((!expr::num::in(workers, 1, std::numeric_limits<int>::max())), "workers out of range") {}
}
scheduler::~scheduler() {
    if(_impl != nullptr)
        await_stop();
}
void scheduler::post(void_func work) const { impl().post(std::move(work)); }

void scheduler::await(native_handle handle, void_func cb) const { impl().await(handle, std::move(cb)); }
void scheduler::await(std::chrono::steady_clock::time_point time_point, void_func cb) const {
    impl().await(time_point, std::move(cb));
}

void scheduler::start() {
    impl().start();
    for(auto& worker : _workers)
        worker = std::jthread(
            [&ctx = impl().ctx, impl = _impl.get(), &count = *_activeWorkers] {
                ++count;
                _threadScheduler = impl;

                ctx.run();
                --count;
            }
        );
}
void scheduler::request_stop() {
    impl().request_stop();
    for(auto& worker : _workers)
        worker.request_stop();
}

bool scheduler::active() const {
    CTH_CRITICAL(!_impl, "use after move") {}
    return impl().active() || _activeWorkers->load() > 0;
}

}


namespace cth::co {
scheduler::scheduler(scheduler&& other) noexcept = default;
scheduler& scheduler::operator=(scheduler&& other) noexcept = default;

}

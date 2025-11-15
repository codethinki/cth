#include "cth/coro/scheduler.hpp"

#include "cth/numeric.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>

namespace bas = boost::asio;

namespace cth::co {
struct scheduler::Impl {
    using guard_t = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    // ReSharper disable once CppNonExplicitConvertingConstructor
    Impl(size_t workers) : ctx{static_cast<int>(workers)} { ctx.stop(); }

    [[nodiscard]] bool active() const { return workGuard != std::nullopt || !ctx.stopped(); }

    void start() {
        ctx.restart();
        workGuard.emplace(boost::asio::make_work_guard(ctx));
    }
    void request_stop() {
        workGuard = std::nullopt;
        ctx.stop();
    }

    bas::io_context ctx;
    std::optional<guard_t> workGuard;
};
}

namespace cth::co {
scheduler::scheduler(size_t workers) :
    _impl{std::make_unique<Impl>(workers)},
    _activeWorkers{std::make_unique<std::atomic<size_t>>()},
    _workers{workers} {
    CTH_CRITICAL(!expr::num::in(workers, 1, std::numeric_limits<int>::max()), "workers out of range") {}
}
scheduler::~scheduler() {
    if(_impl != nullptr)
        await_stop();
}

void scheduler::post(executor_void_task task) {
    bas::post(impl().ctx, [task = std::move(task)]() mutable { task.resume(); });
}
void scheduler::start() {
    impl().start();
    for(auto& worker : _workers)
        worker = std::jthread(
            [&ctx = impl().ctx, &count = *_activeWorkers] {
                ++count;
                ctx.run();
                --count;
            }
        );
}
void scheduler::request_stop() {
    impl().request_stop();
    for(auto& worker : _workers) worker.request_stop();
}
void scheduler::ctx_switch_awaiter::await_suspend(std::coroutine_handle<> h) const {
    bas::post(sched->impl().ctx, [h]() mutable { h.resume(); });
}
bool scheduler::active() const { return impl().active() || _activeWorkers->load() > 0; }


}

namespace cth::co {
scheduler::scheduler(scheduler&& other) noexcept = default;
scheduler& scheduler::operator=(scheduler&& other) noexcept = default;

}

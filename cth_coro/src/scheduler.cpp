#include "cth/coro/scheduler.hpp"

#include "scheduler_impl.hpp"


#include <cth/numeric.hpp>


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
            [&ctx = impl().ctx, impl = _impl.get(), &count = *_activeWorkers, self = this] {
                ++count;
                _threadScheduler = impl;
                _current = self;

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

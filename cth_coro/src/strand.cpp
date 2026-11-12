#include "cth/coro/strand.hpp"

#include "scheduler_impl.hpp"


#include <asio/strand.hpp>


namespace cth::co {

struct strand::Impl {
    co::scheduler const& owner; ///< owns the coroutines detached on the strand
    scheduler::Impl& parent;
    bas::strand<bas::io_context::executor_type> executor;
};
}


namespace cth::co {

strand::strand(scheduler const& scheduler) :
    _impl{std::make_unique<Impl>(scheduler, scheduler.impl(), bas::make_strand(scheduler.impl().ctx))} {}
strand::~strand() = default;

void strand::post(void_func work) const { dev::post_on(impl().executor, scoped(std::move(work))); }

void strand::await(native_handle handle, await_func cb) const {
    dev::await_on(impl().executor, impl().parent.ctx, handle, scoped(std::move(cb)));
}
void strand::await(chrono::time_point_t time_point, await_func cb) const {
    dev::await_on(impl().executor, impl().parent.timerPool, time_point, scoped(std::move(cb)));
}

context const& strand::detach(std::coroutine_handle<> h) const { return impl().owner.detach(h); }
void strand::release(std::coroutine_handle<> h) const { impl().parent.release(h); }

bool strand::owns_thread() const { return impl().executor.running_in_this_thread(); }

}


namespace cth::co {
strand::strand(strand&& other) noexcept = default;
strand& strand::operator=(strand&& other) noexcept = default;

}

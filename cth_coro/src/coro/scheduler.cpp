#include "cth/coro/scheduler.hpp"

#include "cth/coro/utility/exception.hpp"
#include "utility/native_handle_helpers.hpp"

#include <cth/numeric.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/steady_timer.hpp>

namespace bas = boost::asio;
namespace chr = std::chrono;


#define BOOST_EC_STABLE_THROW(ec, msg, ...) \
    CTH_STABLE_THROW_T(cth::except::coro_exception, ec.failed(), msg, __VA_ARGS__) {\
        details->add("message: {}", ec.message());\
        details->add("category: {}", ec.category().name());\
        if(ec.has_location()) {\
            auto const& loc = ec.location();\
            details->add("location: {} ({}, {})", loc.file_name(), loc.line(), loc.column());\
        }\
    } 


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

    void post(void_func work) { bas::post(ctx, std::move(work)); }

    void await(native_handle handle, void_func cb) {
        auto handler = wrap_unique(handle, ctx);
        handler->async_wait(
            [h = std::move(handler), this, f = std::move(cb)](boost::system::error_code const& ec) mutable {
                BOOST_EC_STABLE_THROW(ec, "async wait for handle [{}] failed", h->native_handle())

                post(std::move(f));
            }
        );
    }
    void await(chr::steady_clock::time_point time_point, void_func callback) {
        auto timer = std::make_unique<bas::steady_timer>(ctx, time_point);

        timer->async_wait(
            [this, time_point, t = std::move(timer), cb = std::move(callback)](boost::system::error_code const& ec) mutable {
                BOOST_EC_STABLE_THROW(
                    ec,
                    "async wait for time_point [{}] failed",
                    time_point.time_since_epoch()
                )

                post(std::move(cb));
            }
        );
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
    CTH_CRITICAL((!expr::num::in(workers, 1, std::numeric_limits<int>::max())), "workers out of range") {}
}
scheduler::~scheduler() {
    if(_impl != nullptr)
        await_stop();
}
void scheduler::post(void_func work) { bas::post(impl().ctx, std::move(work)); }

void scheduler::await(native_handle handle, void_func cb) { impl().await(handle, std::move(cb)); }
void scheduler::await(std::chrono::steady_clock::time_point time_point, void_func cb) {
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
    for(auto& worker : _workers) worker.request_stop();
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

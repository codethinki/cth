#pragma once
#include "cth/coro/scheduler.hpp"

#include "utility/asio/ops.hpp"
#include "utility/timer_pool.hpp"


#include <asio/executor_work_guard.hpp>
#include <asio/io_context.hpp>

#include <cth/data/hash_map.hpp>

#include <coroutine>
#include <mutex>
#include <optional>
#include <utility>


namespace cth::co {

struct scheduler::Impl {
    using guard_t = asio::executor_work_guard<asio::io_context::executor_type>;
    using coroutine_set_t = cth::dt::flat_hash_set<std::coroutine_handle<>>;

    // ReSharper disable once CppNonExplicitConvertingConstructor
    Impl(size_t workers) : ctx{static_cast<int>(workers)}, timerPool{ctx} { ctx.stop(); }

    // the destroy in ~Impl(), not in ~scheduler(): the move assignment destroys the old Impl too
    /**
     * destroys the detached coroutines that did not end
     * @details then the members: the asio context drops the jobs that did not run, it calls none of them
     * @pre no worker runs
     */
    ~Impl() {
        // out of the lock: the destruction of a coroutine can call release() or detach()
        coroutine_set_t coroutines{};
        {
            std::scoped_lock const lock{detachedMutex};
            coroutines = std::exchange(detached, {});
        }
        for(auto const h : coroutines)
            h.destroy();
    }

    /**
     * takes the ownership of the coroutine
     * @see `context::detach()`
     */
    void detach(std::coroutine_handle<> h) {
        std::scoped_lock const lock{detachedMutex};
        detached.insert(h);
    }

    /**
     * releases the coroutine to the caller
     * @see `context::release()`
     */
    void release(std::coroutine_handle<> h) {
        std::scoped_lock const lock{detachedMutex};
        detached.erase(h);
    }

    [[nodiscard]] bool active() const { return workGuard != std::nullopt || !ctx.stopped(); }

    void start() {
        ctx.restart();
        workGuard.emplace(asio::make_work_guard(ctx));
    }
    void request_stop() {
        workGuard = std::nullopt;
        ctx.stop();
    }


    asio::io_context ctx;
    std::optional<guard_t> workGuard;

    timer_pool timerPool;

    std::mutex detachedMutex;
    coroutine_set_t detached; ///< the detached coroutines that did not end, `~Impl()` destroys them
};
}

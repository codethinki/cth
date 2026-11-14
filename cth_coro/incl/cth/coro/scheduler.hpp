#pragma once

#include <cth/io/log.hpp>
#include <cth/chrono.hpp>

#include "context.hpp"
#include "utility/fwd.hpp"


#include <thread>
#include <vector>


namespace cth::co {
/**
 * tag, the `scheduler` constructor with it calls `scheduler::start()`
 */
constexpr auto autostart = [] {};
using autostart_t = decltype(autostart);
}

namespace cth::co {
/**
 * runs its jobs on its own worker threads
 * @details `context` on an `asio::io_context`
 * @details call `start()` to run the workers
 */
class scheduler final : public context {
    struct Impl;
    friend class strand;

    static inline thread_local Impl* _threadScheduler = nullptr;

public:
    // ReSharper disable once CppNonExplicitConvertingConstructor
    /**
     * constructs the scheduler, does not start the workers
     * @param workers number of worker threads
     * @pre `0 < workers < MAX_INT`
     */
    scheduler(size_t workers = 1);

    // ReSharper disable once CppNonExplicitConvertingConstructor
    /**
     * constructs the scheduler and calls `start()`
     * @param workers number of worker threads
     * @pre `0 < workers < MAX_INT`
     */
    scheduler(autostart_t, size_t workers = 1) : scheduler{workers} { start(); }
    /**
     * calls `await_stop()`, destroys the detached coroutines that did not end
     * @details blocks until the jobs that run end
     * @note jobs that did not start do not run
     * @attention destroys also a detached coroutine that waits on another context, that context can still resume it
     */
    ~scheduler();

    void post(void_func work) const override;

    void await(native_handle, await_func cb) const override;
    void await(chrono::time_point_t time_point, await_func cb) const override;

    [[nodiscard]] context const& detach(std::coroutine_handle<>) const override;
    void release(std::coroutine_handle<>) const override;

    /**
     * starts the workers
     * @pre not `active()`
     * @post `active()`
     */
    void start();

    /**
     * tells the workers to stop, does not wait for them
     * @details jobs that run end, jobs that did not start run after the next `start()`
     * @note call `await_stop()` to wait for the workers
     */
    void request_stop();

    /**
     * calls `request_stop()` and blocks until the workers end
     */
    void await_stop() {
        request_stop();
        for(auto& worker : _workers)
            if(worker.joinable())
                worker.join();
    }

private:
    [[nodiscard]] Impl& impl() const { return *_impl; }

    std::unique_ptr<Impl> _impl;
    std::unique_ptr<std::atomic<size_t>> _activeWorkers{};
    std::vector<std::jthread> _workers{};

public:
    /**
     * checks if the calling thread is a worker of this scheduler
     * @details also `true` in the jobs of its strands
     */
    [[nodiscard]] bool owns_thread() const override { return _threadScheduler == _impl.get(); }

    [[nodiscard]] size_t workers() const { return _workers.size(); }
    /**
     * counts the workers that run at the moment
     * @pre not moved from
     */
    [[nodiscard]] size_t active_workers() const {
        CTH_CRITICAL(!_activeWorkers, "use after move") {}
        return _activeWorkers->load(std::memory_order_relaxed);
    }
    /**
     * checks if the scheduler runs or a worker did not end yet
     * @pre not moved from
     */
    [[nodiscard]] bool active() const;

    scheduler(scheduler const& other) = delete;
    scheduler& operator=(scheduler const& other) = delete;
    /**
     * moves `other` into this scheduler
     * @pre `other` not `active()`
     * @attention executors and coroutines of `other` still refer to `other`
     */
    scheduler(scheduler&& other) noexcept;
    /**
     * moves `other` into this scheduler
     * @pre not `active()`
     * @pre `other` not `active()`
     * @note destroys the detached coroutines of this scheduler that did not end
     * @attention executors and coroutines of `other` still refer to `other`
     */
    scheduler& operator=(scheduler&& other) noexcept;
};

}

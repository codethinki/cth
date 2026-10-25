#pragma once

#include <cth/io/log.hpp>
#include <cth/chrono.hpp>

#include "context.hpp"
#include "utility/fwd.hpp"


#include <thread>
#include <vector>


namespace cth::co {
constexpr auto autostart = [] {};
using autostart_t = decltype(autostart);
}

namespace cth::co {
/**
 * runs its jobs on its own worker threads
 * @details @ref context on an asio::io_context
 */
class scheduler final : public context {
    struct Impl;
    friend class strand;

    static inline thread_local Impl* _threadScheduler = nullptr;

public:
    // ReSharper disable once CppNonExplicitConvertingConstructor
    /**
     * constructs the scheduler with workers
     * @param workers to process the waits with
     * @pre 0 < @ref workers < MAX_INT
     */
    scheduler(size_t workers = 1);

    // ReSharper disable once CppNonExplicitConvertingConstructor
    /**
     * constructs the scheduler with workers and calls @ref start()
     * @param workers to process the waits with
     * @pre 0 < @ref workers < MAX_INT
     */
    scheduler(autostart_t, size_t workers = 1) : scheduler{workers} { start(); }
    /**
     * stops and destroys, may block until the current worker tasks finish
     */
    ~scheduler();

    void post(void_func work) const override;

    void await(native_handle, await_func cb) const override;
    void await(chrono::time_point_t time_point, await_func cb) const override;


    void start();

    void request_stop();

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
    [[nodiscard]] bool owns_thread() const override { return _threadScheduler == _impl.get(); }

    [[nodiscard]] size_t workers() const { return _workers.size(); }
    [[nodiscard]] size_t active_workers() const {
        CTH_CRITICAL(!_activeWorkers, "use after move") {}
        return _activeWorkers->load(std::memory_order_relaxed);
    }
    [[nodiscard]] bool active() const;

    scheduler(scheduler const& other) = delete;
    scheduler& operator=(scheduler const& other) = delete;
    /**
     * @param other must not be @ref active()
     */
    scheduler(scheduler&& other) noexcept;
    /**
     * @param other must not be @ref active()
     * @pre must not be @ref active()
     */
    scheduler& operator=(scheduler&& other) noexcept;
};

}

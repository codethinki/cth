#pragma once
#include "executor_task.hpp"
#include "utility.hpp"

#include <vector>
#include <thread>


namespace cth::co {
class scheduler {
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


    void post(executor_void_task task);

    void start();

    void request_stop();

    void await_stop() {
        request_stop();
        for(auto& worker : _workers)
            if(worker.joinable())
                worker.join();
    }

    struct ctx_switch_awaiter {
        scheduler* sched;

        bool await_ready() const noexcept { return false; }

        void await_suspend(std::coroutine_handle<> h) const;

        void await_resume() const noexcept {}
    };

    ctx_switch_awaiter transfer_exec() { return {this}; }

private:
    struct Impl;
    // ReSharper disable once CppMemberFunctionMayBeConst
    [[nodiscard]] Impl& impl() { return *_impl; }
    [[nodiscard]] Impl const& impl() const { return *_impl; }

    std::unique_ptr<Impl> _impl;
    std::unique_ptr<std::atomic<size_t>> _activeWorkers{};
    std::vector<std::jthread> _workers{};

public:
    [[nodiscard]] size_t workers() const { return _workers.size(); }
    [[nodiscard]] size_t active_workers() const { return _activeWorkers->load(); }
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

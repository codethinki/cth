#pragma once

#include "cth/io/log.hpp"

#include "os/native_handle.hpp"

#include "utility/fwd.hpp"

#include <functional>
#include <thread>
#include <vector>


namespace cth::co {
constexpr auto autostart = [] {};
using autostart_t = decltype(autostart);
}


namespace cth::co {
class scheduler {
    struct Impl;

    static inline thread_local Impl* _threadScheduler = nullptr;

public:
    using native_handle = cth::co::os::native_handle_t;
    using void_func = std::move_only_function<void()>;

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

    void post(void_func work);

    void await(native_handle, void_func cb);
    void await(time_point_t time_point, void_func cb);


    void start();

    void request_stop();

    void await_stop() {
        request_stop();
        for(auto& worker : _workers)
            if(worker.joinable())
                worker.join();
    }

private:
    // ReSharper disable once CppMemberFunctionMayBeConst
    [[nodiscard]] Impl& impl() { return *_impl; }
    [[nodiscard]] Impl const& impl() const { return *_impl; }

    std::unique_ptr<Impl> _impl;
    std::unique_ptr<std::atomic<size_t>> _activeWorkers{};
    std::vector<std::jthread> _workers{};

public:
    [[nodiscard]] bool owns_thread() const { return _threadScheduler == _impl.get(); }

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

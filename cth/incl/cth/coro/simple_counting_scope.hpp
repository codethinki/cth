#pragma once
#include <cth/io/log.hpp>

#include <atomic>
#include <cstddef>


namespace cth::co {
/**
 * counter of the running detached tasks
 * @details
 * - a detached task enters it at its start
 * - the task leaves it at the end of the destruction of its coroutine, only the deallocation of the frame follows
 * - `wait()` blocks until no task runs
 * - thread safe
 * @see `executor::detach()`
 */
class simple_counting_scope {
public:
    simple_counting_scope() = default;

    /**
     * @pre no running task: call `wait()` first
     */
    ~simple_counting_scope() {
        CTH_CRITICAL(_count.load() != 0, "the scope must have no running tasks, got {}", _count.load()) {}
    }

    /**
     * counts a task that starts
     */
    void enter() noexcept { _count.fetch_add(1, std::memory_order_relaxed); }

    /**
     * counts a task that ended
     * @details the last task wakes the waiters
     * @pre the parts of the coroutine of the task are destroyed, the deallocation of its frame can follow
     */
    void leave() noexcept {
        // release: the destruction of the coroutine of the task happens before wait() returns
        if(_count.fetch_sub(1, std::memory_order_release) != 1) return;

        //TODO technically UB: wait() can see 0 before notify_all() runs, the owner destroys the scope, notify_all()
        // then runs on a destroyed atomic. in practice notify_all() uses the address only as a key (futex,
        // WakeByAddressAll), libstdc++ std::latch::count_down() does the same. fix it once std offers a way (P2616)
        _count.notify_all();
    }

    /**
     * blocks until no task runs
     * @details no task -> returns at once
     * @attention blocks the calling thread
     */
    void wait() const noexcept {
        for(auto count = _count.load(std::memory_order_acquire); count != 0;
            count = _count.load(std::memory_order_acquire))
            _count.wait(count, std::memory_order_acquire);
    }

private:
    std::atomic<size_t> _count{};

public:
    simple_counting_scope(simple_counting_scope const& other) = delete;
    simple_counting_scope(simple_counting_scope&& other) = delete;
    simple_counting_scope& operator=(simple_counting_scope const& other) = delete;
    simple_counting_scope& operator=(simple_counting_scope&& other) = delete;
};
}

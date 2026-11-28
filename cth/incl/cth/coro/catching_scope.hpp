#pragma once
#include "cth/coro/simple_counting_scope.hpp"

#include <cth/io/log.hpp>
#include <cth/types.hpp>

#include <atomic>
#include <cstdint>
#include <exception>
#include <utility>


namespace cth::co {
/**
 * counter of the running detached tasks, keeps the first error of them
 * @details
 * - a task of `executor::detach()` in the scope: an escaped exception is stored, it does not terminate
 * - owns its tasks: `~scheduler()` destroys only the tasks without a scope, call `wait()` before it
 * - the first `store()` wins, the later errors are dropped
 * - thread safe: `enter()`, `leave()`, `wait()`, `store()` and `failed()` from any thread
 * @see `simple_counting_scope`
 */
class catching_scope {
public:
    catching_scope() = default;

    /**
     * @pre no running task: call `wait()` first
     */
    ~catching_scope() = default;

    /**
     * counts a task that starts
     */
    void enter() noexcept { _tasks.enter(); }

    /**
     * counts a task that ended
     * @pre the coroutine of the task is destroyed, up to the deallocation of its frame
     */
    void leave() noexcept { _tasks.leave(); }

    /**
     * blocks until no task runs
     * @details the error of a task that ended is visible afterwards
     * @attention blocks the calling thread
     */
    void wait() const noexcept { _tasks.wait(); }

    /**
     * stores the error if it is the first
     * @details a later error is dropped
     * @param error to store
     */
    void store(std::exception_ptr error) noexcept {
        auto expected = error_state::NONE;
        // the winner is the only writer of the error, the losers do not read it
        if(!_state.compare_exchange_strong(expected, error_state::WRITING, std::memory_order_relaxed))
            return;

        _error = std::move(error);
        // release: a `failed()` that sees the state sees the error
        _state.store(error_state::STORED, std::memory_order_release);
    }

    /**
     * @return an error is stored
     */
    [[nodiscard]] bool failed() const noexcept {
        return _state.load(std::memory_order_acquire) == error_state::STORED;
    }

    /**
     * rethrows the stored error
     * @pre `failed()`
     */
    void rethrow() const {
        CTH_CRITICAL(!failed(), "rethrow() needs a stored error") {}
        std::rethrow_exception(_error);
    }

    /**
     * forgets the error, the next `store()` stores the first error again
     * @pre no running task and no concurrent `store()`
     */
    void clear() noexcept {
        _error = nullptr;
        _state.store(error_state::NONE, std::memory_order_release);
    }

private:
    enum class error_state : u8 {
        NONE,
        WRITING, ///< a `store()` writes the error
        STORED,
    };

    simple_counting_scope _tasks{};
    std::atomic<error_state> _state{error_state::NONE};
    std::exception_ptr _error = nullptr; ///< written once per `clear()`, by the first `store()`

public:
    catching_scope(catching_scope const& other) = delete;
    catching_scope(catching_scope&& other) = delete;
    catching_scope& operator=(catching_scope const& other) = delete;
    catching_scope& operator=(catching_scope&& other) = delete;
};
}

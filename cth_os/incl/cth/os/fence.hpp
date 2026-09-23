#pragma once

#include "cth/os/native_handle.hpp"
#include "cth/os/wait.hpp"

#include <chrono>


namespace cth::os {

/**
 * Manual-reset, OS-backed synchronization fence.
 *
 * A signaled fence releases every current waiter and remains signaled until
 * @ref reset() is called. Its native handle can be registered with an OS wait
 * service without dedicating a thread to @ref wait().
 *
 * The fence owns its native handle. A handle returned by @ref native_handle()
 * is borrowed and remains valid until the fence is moved from or destroyed.
 */
class fence {
public:
    /** creates a fence with the requested initial state */
    explicit fence(bool signaled = false);
    ~fence() = default;

    /** signals the fence and releases every waiter */
    void signal() noexcept;

    /** clears the signaled state */
    void reset() noexcept;

    /**
     * blocks until signaled or timed out
     * @return the result of the OS wait operation
     */
    wait_result wait(std::chrono::milliseconds timeout = INFINITE_WAIT) const; // NOLINT(*-use-nodiscard)

private:
    static unique_native_handle create_handle(bool signaled);

    unique_native_handle _handle;

public:
    /** checks the current state without changing it */
    [[nodiscard]] bool signaled() const;

    /**
     * @return the borrowed OS handle used to wait for this fence
     * @attention the handle is invalidated when the fence is moved from or destroyed
     */
    [[nodiscard]] native_handle_t native_handle() const noexcept { return _handle.get(); }

    fence(fence const&) = delete;
    fence& operator=(fence const&) = delete;
    fence(fence&&) noexcept = default;
    fence& operator=(fence&&) noexcept = default;
};

}

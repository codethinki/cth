#pragma once
#include <cth/chrono.hpp>
#include <cth/os/native_handle.hpp>

#include <functional>
#include <utility>


namespace cth::co {
class context;

/**
 * @return context whose job runs on the calling thread, nullptr -> no job of a context
 */
[[nodiscard]] context const* current_context() noexcept;

/**
 * where coroutines run and wait
 * @details implemented by @ref scheduler, @ref executor and the awaiters only use this interface
 */
class context {
public:
    using void_func = std::move_only_function<void()>;
    using native_handle = os::native_handle_t;

    /**
     * runs the work on this context
     */
    virtual void post(void_func work) const = 0;

    /**
     * calls the callback on this context once the handle is signaled
     * @param cb to call
     */
    virtual void await(native_handle, void_func cb) const = 0;
    /**
     * calls the callback on this context once the time point is reached
     * @param time_point to wait for
     * @param cb to call
     */
    virtual void await(chrono::time_point_t time_point, void_func cb) const = 0;

    /**
     * true if the calling thread runs a job of this context
     * @details a scheduler also owns the threads that run the jobs of its strands
     */
    [[nodiscard]] virtual bool owns_thread() const = 0;

protected:
    context() = default;
    ~context() = default;

    context(context const& other) = default;
    context& operator=(context const& other) = default;
    context(context&& other) noexcept = default;
    context& operator=(context&& other) noexcept = default;

    /**
     * wraps the work: @ref current_context() is this context while the work runs
     * @details restores the previous context after the work, also if the work throws
     */
    [[nodiscard]] auto scoped(void_func work) const {
        return [self = this, work = std::move(work)]() mutable {
            current_guard const guard{self};
            work();
        };
    }

    /**
     * context of the job that runs on this thread, see @ref current_context()
     */
    static inline thread_local context const* _current = nullptr;

private:
    struct current_guard {
        explicit current_guard(context const* ctx) noexcept : previous{std::exchange(_current, ctx)} {}
        ~current_guard() { _current = previous; }

        context const* previous;

        current_guard(current_guard const&) = delete;
        current_guard& operator=(current_guard const&) = delete;
    };

    friend context const* current_context() noexcept;
};

inline context const* current_context() noexcept { return context::_current; }
}

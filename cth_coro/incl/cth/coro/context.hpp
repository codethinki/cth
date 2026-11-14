#pragma once
#include <cth/chrono.hpp>
#include <cth/os/native_handle.hpp>

#include <coroutine>
#include <functional>
#include <system_error>
#include <utility>


namespace cth::co {
/**
 * interface of where coroutines run and wait
 * @details implemented by `scheduler` and `strand`, `executor` and the awaiters only use this interface
 */
class context {
public:
    using void_func = std::move_only_function<void()>;
    using await_func = std::move_only_function<void(std::error_code)>;
    using native_handle = os::native_handle_t;

    /**
     * posts the work to this context
     * @param work to run
     * @attention work that throws terminates the program
     */
    virtual void post(void_func work) const = 0;

    /**
     * calls the callback on this context once the handle is signaled
     * @details the callback gets the result of the wait, an empty `error_code` -> signaled
     * @param cb to call
     * @throws cth::except::os_exception reason: the handle cannot be duplicated
     * @attention a callback that throws terminates the program
     */
    virtual void await(native_handle, await_func cb) const = 0;
    /**
     * calls the callback on this context once the time point is reached
     * @details the callback gets the result of the wait, an empty `error_code` -> reached
     * @param time_point to wait for
     * @param cb to call
     * @attention a callback that throws terminates the program
     */
    virtual void await(chrono::time_point_t time_point, await_func cb) const = 0;

    /**
     * gives the ownership of the coroutine to the scheduler of this context
     * @details the coroutine did not end at the destruction of the scheduler -> the scheduler destroys it
     * @return the owner: a scheduler itself, a strand its scheduler
     * @see `release()`
     */
    [[nodiscard]] virtual context const& detach(std::coroutine_handle<>) const = 0;

    /**
     * releases the coroutine to the caller, does not destroy it
     * @pre `detach()` of the coroutine returned this context
     */
    virtual void release(std::coroutine_handle<>) const = 0;

    /**
     * checks if the calling thread runs a job of this context
     * @details a scheduler also owns the threads that run the jobs of its strands
     */
    [[nodiscard]] virtual bool owns_thread() const = 0;

    /**
     * gets the context whose job runs on the calling thread
     * @return context of the job, `nullptr` -> no job of a context
     */
    [[nodiscard]] static context const* current() noexcept { return _current; }

protected:
    context() = default;
    ~context() = default;

    context(context const& other) = default;
    context& operator=(context const& other) = default;
    context(context&& other) noexcept = default;
    context& operator=(context&& other) noexcept = default;

    /**
     * wraps the work: `context::current()` is this context while the work runs
     * @details restores the previous context after the work, also if the work throws
     * @details forwards the arguments of the call to the work
     * @param work to wrap
     * @return wrapped work
     */
    template<class Work>
    [[nodiscard]] auto scoped(Work work) const {
        return [self = this, work = std::move(work)]<class... Args>(Args&&... args) mutable {
            current_guard const guard{self};
            work(std::forward<Args>(args)...);
        };
    }

    /**
     * context of the job that runs on this thread
     * @see `context::current()`
     */
    static inline thread_local context const* _current = nullptr;

private:
    /**
     * sets `_current` for its lifetime, restores the previous value
     */
    struct current_guard {
        explicit current_guard(context const* ctx) noexcept : previous{std::exchange(_current, ctx)} {}
        ~current_guard() { _current = previous; }

        context const* previous;

        current_guard(current_guard const&) = delete;
        current_guard& operator=(current_guard const&) = delete;
    };
};
}

namespace cth::co::dev {
/**
 * gives the coroutine to continue with on the context, for a symmetric transfer
 * @details `context::current()` is the target -> no post, the caller transfers to the coroutine or resumes it
 * @param target context the coroutine continues on
 * @param h coroutine to continue
 * @return the coroutine if `context::current()` is the target, else `std::noop_coroutine()` after a post of the resume
 * @attention nothing may touch the frame of the caller after the post: the coroutine can resume at once
 */
[[nodiscard]] inline std::coroutine_handle<> transfer_on(context const& target, std::coroutine_handle<> h) {
    if(context::current() == &target) return h;
    target.post([h] { h.resume(); });
    return std::noop_coroutine();
}
}

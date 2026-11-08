#pragma once
#include <cth/meta/coro.hpp>

#include <coroutine>
#include <exception>
#include <utility>

namespace cth::co::dev {

/**
 * awaiter that starts a task and gives its result
 * @tparam Promise promise of the task
 */
template<cth_promise Promise>
struct basic_promise_awaiter {
    using promise_type = Promise;

    constexpr basic_promise_awaiter(std::coroutine_handle<promise_type> h) : handle{h} {}

    std::coroutine_handle<promise_type> handle; ///< coroutine of the task, the task owns it

    /**
     * ready if there is no coroutine or the task is done
     */
    [[nodiscard]] constexpr bool await_ready() const noexcept { return !handle || handle.done(); }

    /**
     * stores the caller as continuation and starts the task
     * @param caller coroutine of the caller
     * @return coroutine of the task
     */
    [[nodiscard]] auto await_suspend(std::coroutine_handle<> caller) noexcept
        -> std::coroutine_handle<promise_type> {
        handle.promise().continuation = caller;
        return handle;
    }

    /**
     * gives the result of the task
     * @throws what the task throws
     * @note moves the result out
     */
    [[nodiscard]] auto await_resume() -> promise_value_type<Promise> {
        using return_value = promise_value_type<Promise>;

        if(handle.promise().exception)
            std::rethrow_exception(handle.promise().exception);

        if constexpr(!std::same_as<void, return_value>)
            return std::forward<return_value>(*handle.promise().result);
        else
            return;
    }
};

}

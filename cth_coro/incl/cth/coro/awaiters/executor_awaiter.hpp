#pragma once
#include "executor_awaiter_base.hpp"

#include <cth/types/coro.hpp>
#include <exception>

namespace cth::co {
template<cth_promise Promise>
struct executor_awaiter : executor_awaiter_base {
    using promise_type = Promise;

    executor_awaiter(std::coroutine_handle<promise_type> h) : handle{h} {}


    [[nodiscard]] bool await_ready() const noexcept { return !handle || handle.done(); }

    template<class T>
    auto await_suspend(std::coroutine_handle<T> caller) noexcept -> std::coroutine_handle<promise_type> {
        executor_awaiter_base::inject_executor(handle);
        handle.promise().continuation = caller;
        return handle;
    }

    auto await_resume() {
        if(handle.promise().exception)
            std::rethrow_exception(handle.promise().exception);
        if constexpr(!std::same_as<void, promise_value_type<promise_type>>)
            return std::move(*handle.promise().result);
    }

    std::coroutine_handle<promise_type> handle;
};
}

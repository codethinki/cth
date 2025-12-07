#pragma once
#include <cth/types/coro.hpp>

#include <coroutine>
#include <exception>

namespace cth::co::dev {

template<cth_promise Promise>
struct basic_awaiter {
    using promise_type = Promise;

    basic_awaiter(std::coroutine_handle<promise_type> h) : handle{h} {}

    std::coroutine_handle<promise_type> handle;

    [[nodiscard]] bool await_ready() const noexcept { return !handle || handle.done(); }

    auto await_suspend(std::coroutine_handle<> caller) noexcept -> std::coroutine_handle<> {
        handle.promise().continuation = caller;
        return handle;
    }

    auto await_resume() {
        if(handle.promise().exception)
            std::rethrow_exception(handle.promise().exception);
        if constexpr(!std::same_as<void, promise_value_type<Promise>>)
            return std::move(*handle.promise().result);
    }
};

}

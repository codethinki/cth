#pragma once
#include <cth/meta/coro.hpp>

#include <coroutine>
#include <exception>
#include <utility>

namespace cth::co::dev {

template<cth_promise Promise>
struct basic_awaiter {
    using promise_type = Promise;

    basic_awaiter(std::coroutine_handle<promise_type> h) : handle{h} {}

    std::coroutine_handle<promise_type> handle;

    [[nodiscard]] bool await_ready() const noexcept { return !handle || handle.done(); }

    auto await_suspend(std::coroutine_handle<> caller) noexcept -> std::coroutine_handle<promise_type> {
        handle.promise().continuation = caller;
        return handle;
    }

    promise_value_type<Promise> await_resume() {
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

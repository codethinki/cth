#pragma once
#include "cth/coro/awaiters/dev/final_awaiter.hpp"
#include "cth/coro/tasks/promises/dev/result_storage.hpp"

#include <coroutine>

namespace cth::co {
template<class T>
struct basic_promise {
    using value_type = T;

    dev::result_storage<T> result;
    std::exception_ptr exception;
    std::coroutine_handle<> continuation = nullptr;

    std::suspend_always initial_suspend() noexcept { return {}; }
    dev::final_awaiter final_suspend() noexcept { return {}; }

    void unhandled_exception() { exception = std::current_exception(); }

    void return_value(T&& value) noexcept { result.emplace(std::move(value)); }
    void return_value(T const& value) noexcept requires std::copy_constructible<T> { result.emplace(value); }
};

template<>
struct basic_promise<void> {
    using value_type = void;

    dev::result_storage<void> result;
    std::exception_ptr exception;
    std::coroutine_handle<> continuation = nullptr;

    std::suspend_always initial_suspend() noexcept { return {}; }
    dev::final_awaiter final_suspend() noexcept { return {}; }

    void unhandled_exception() { exception = std::current_exception(); }

    void return_void() noexcept { result.emplace(); }
};
}

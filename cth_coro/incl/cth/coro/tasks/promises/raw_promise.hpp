#pragma once
#include <cth/data/optional.hpp>

#include <coroutine>


namespace cth::co {
template<class T>
struct raw_promise {
    using value_type = T;

    dt::optional<T> result;
    std::exception_ptr exception;
    std::coroutine_handle<> continuation = nullptr;

    void unhandled_exception() { exception = std::current_exception(); }

    template<class U>
    void return_value(U&& value) noexcept {
        result.emplace(std::forward<U>(value));
    }
};

template<>
struct raw_promise<void> {
    using value_type = void;

    dt::optional<void> result;
    std::exception_ptr exception;
    std::coroutine_handle<> continuation = nullptr;

    void unhandled_exception() { exception = std::current_exception(); }

    void return_void() noexcept { result.emplace(); }
};
}

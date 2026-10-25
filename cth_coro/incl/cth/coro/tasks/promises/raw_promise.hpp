#pragma once
#include "cth/coro/utility/fwd.hpp"

#include <cth/data/optional.hpp>
#include <cth/meta/concepts.hpp>

#include <coroutine>
#include <exception>
#include <utility>


namespace cth::co::dev {
/**
 * base of every @ref raw_promise: the exception and the continuation
 */
struct raw_promise_base {
    /**
     * exception of the task, nullptr -> no exception
     */
    std::exception_ptr exception;
    /**
     * coroutine that continues after the task, nullptr -> none
     */
    std::coroutine_handle<> continuation = nullptr;

    /**
     * stores the current exception
     */
    constexpr void unhandled_exception() { exception = std::current_exception(); }
};
}

namespace cth::co {
/**
 * promise part that stores the result and the exception
 * @tparam T result type
 */
template<class T>
struct raw_promise : dev::raw_promise_base {
    using value_type = T;

    dt::optional<T> result;

    /**
     * stores the result
     * @throws what the construction of T throws
     */
    template<class U>
    constexpr void return_value(U&& value) noexcept(mta::nothrow_constructible<T, U&&>) {
        result.emplace(std::forward<U>(value));
    }
};

/**
 * promise part of tasks without a result
 */
template<>
struct raw_promise<void> : dev::raw_promise_base {
    using value_type = void;

    dt::optional<void> result;

    constexpr void return_void() noexcept { result.emplace(); }
};
}

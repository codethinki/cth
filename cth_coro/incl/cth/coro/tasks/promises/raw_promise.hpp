#pragma once
#include "cth/coro/utility/fwd.hpp"

#include <cth/data/optional.hpp>
#include <cth/meta/concepts.hpp>

#include <coroutine>
#include <exception>
#include <utility>


namespace cth::co::dev {
/**
 * members of every @ref raw_promise: the exception and the continuation
 */
struct raw_promise_base {
    std::exception_ptr exception;
    std::coroutine_handle<> continuation = nullptr;

    constexpr void unhandled_exception() { exception = std::current_exception(); }
};
}

namespace cth::co {
template<class T>
struct raw_promise : dev::raw_promise_base {
    using value_type = T;

    dt::optional<T> result;

    template<class U>
    constexpr void return_value(U&& value) noexcept(mta::nothrow_constructible<T, U&&>) {
        result.emplace(std::forward<U>(value));
    }
};

template<>
struct raw_promise<void> : dev::raw_promise_base {
    using value_type = void;

    dt::optional<void> result;

    constexpr void return_void() noexcept { result.emplace(); }
};
}

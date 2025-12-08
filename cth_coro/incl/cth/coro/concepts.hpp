#pragma once
#include <cth/types/coro.hpp>

#include <type_traits>

namespace cth::co {
struct executor_promise_base;
struct executor_awaiter_base;
struct capture_awaiter_base;


template<class T>
concept executor_awaitable = std::is_base_of_v<executor_awaiter_base, std::remove_cvref_t<awaiter_t<T>>>;

template<class T>
concept captured_awaitable = co::awaitable<T> && std::is_base_of_v<capture_awaiter_base, co::awaiter_t<T>>;

template<class Promise>
concept executor_injectable = std::is_base_of_v<executor_promise_base, Promise>;

template<class T>
concept foreign_awaitable = co::awaitable<T> && !captured_awaitable<T> && !executor_awaitable<T>;

template<class T>
concept executorless_awaitable = awaitable<T> && !executor_awaitable<T>;

}

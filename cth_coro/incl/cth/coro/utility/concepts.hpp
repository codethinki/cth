#pragma once
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <type_traits>


namespace cth::co::this_coro {}


namespace cth::co {

/**
 * true if the awaiter of @ref T derives @ref this_coro_awaiter_base
 * @details the awaiter resumes the caller on its context itself
 */
template<class T>
concept this_coro_awaitable =
    co::awaitable<T> && std::is_base_of_v<this_coro_awaiter_base, std::remove_cvref_t<co::awaiter_t<T>>>;

/**
 * true if @ref T is awaitable, not a @ref this_coro_awaitable and not a tag
 */
template<class T>
concept foreign_awaitable = co::awaitable<T> && !this_coro_awaitable<T> && !co::this_coro::tag<T>;

template<class T>
concept sync_promise_type = std::is_base_of_v<sync_promise_base, T>;

}

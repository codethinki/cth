#pragma once
#include "cth/coro/utility/fwd.hpp"

#include <cth/meta/coro.hpp>

#include <type_traits>


namespace cth::co::this_coro {

template<class Tag>
concept tag = std::is_base_of_v<tag_base, cth::mta::rcvr_t<Tag>>;
}


namespace cth::co {


template<class T>
concept this_coro_awaitable = std::is_base_of_v<this_coro_awaiter_base, std::remove_cvref_t<awaiter_t<T>>>;

template<class T>
concept captured_awaitable = co::awaitable<T> && std::is_base_of_v<capture_awaiter_base, co::awaiter_t<T>>;

template<class Promise>
concept this_coro_supported_promise = std::is_base_of_v<this_coro_promise_base, Promise>;

template<class T>
concept foreign_awaitable =
    co::awaitable<T>
    && !captured_awaitable<T>
    && !this_coro_awaitable<T>
    && !co::this_coro::tag<T>;

template<class T>
concept non_this_coro_awaitable = awaitable<T> && !this_coro_awaitable<T>;

template<class T>
concept sync_promise_type = std::is_base_of_v<sync_promise_base, T>;

}

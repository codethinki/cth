#pragma once
#include "cth/meta/coro.hpp"

#include <utility>


namespace cth::co {

struct signaled_t {};
inline constexpr signaled_t signaled{};

}

namespace cth::co {
template<awaitable T>
decltype(auto) extract_awaiter(T&& t) {
    if constexpr(!task<T>) return std::forward<T>(t);
    else if constexpr(has_op_co_await<T>) return std::forward<T>(t).operator co_await();
    else if constexpr(has_free_co_await<T>) return operator co_await(std::forward<T>(t));
    else static_assert(false, "library bug: awaitable but unknow awaiter extraction");
}

}

#pragma once

#include <type_traits>

#include <cth/types/coro.hpp>

namespace cth::co::dev {

struct capture_awaiter_base {};

template<class T>
concept captured_awaitable = awaitable<T>
    && std::is_base_of_v<capture_awaiter_base, awaiter_t<T>>;

}

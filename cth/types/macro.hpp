#pragma once
#include "../macro.hpp"

#include <limits>
#include <type_traits>

//independent constants

namespace cth::type {

constexpr auto no_range = [] {};
constexpr auto empty = [] {};
constexpr size_t MAX_DEPTH = (std::numeric_limits<size_t>::max)();


using no_range_t = decltype(no_range);
using empty_t = decltype(empty);

}


namespace cth::type {

template<class T = size_t>
size_t cval zero() { return 0; }

template<class T>
size_t cval zero(T) { return zero<T>(); }
template<auto Val>
size_t cval zero() { return type::zero(Val); }

template<bool Copy, class T>
dclauto copy_if(T&& value) {
    if constexpr(Copy) return std::remove_cvref_t<T>{value};
    else return std::forward<T>(value);
}

}


namespace cth::type {
template<class T>
using pure_t = std::decay_t<T>;
}


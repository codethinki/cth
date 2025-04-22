module;
#include "../macro.hpp"

export module cth.typ.utility;

import std;

export namespace cth::type {

cxpr auto no_range = [] {};
cxpr auto empty = [] {};
cxpr size_t MAX_DEPTH = (std::numeric_limits<size_t>::max)();


using no_range_t = decltype(no_range);
using empty_t = decltype(empty);

}


export namespace cth::type {

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


export namespace cth::type {
template<class T>
using pure_t = std::decay_t<T>;
}
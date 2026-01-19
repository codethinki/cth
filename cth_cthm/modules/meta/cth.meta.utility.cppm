module;
#include "cthm/macro/utility.hpp"

#include <limits>
#include <numeric>
#include <type_traits>
#include <utility>

export module cth.meta.utility;


//independent constants

export namespace cth::mta {

constexpr auto no_range = [] {};
constexpr auto empty = [] {};
constexpr size_t MAX_DEPTH = (std::numeric_limits<size_t>::max)();


using no_range_t = decltype(no_range);
using empty_t = decltype(empty);


template<auto TypeIdentity>
using identity_t = decltype(TypeIdentity)::type;
}


export namespace cth::mta {

template<class T = size_t>
size_t cval zero() { return 0; }

template<class T>
size_t cval zero(T) { return zero<T>(); }
template<auto Val>
size_t cval zero() { return mta::zero(Val); }

template<bool Copy, class T>
declauto copy_if(T&& value) {
    if constexpr(Copy) return std::remove_cvref_t<T>{value};
    else return std::forward<T>(value);
}

}


export namespace cth::mta {
template<class T>
using pure_t = std::decay_t<T>;
}

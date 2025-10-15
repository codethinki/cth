#pragma once
#include "macro.hpp"

#include "types/typ_ranges.hpp"

#include <functional>
#include <boost/pfr.hpp>

namespace cth::hash {
// from: https://stackoverflow.com/a/57595105
namespace dev {
    template<typename T, typename... Rest>
    cxpr void combine(std::size_t& seed, T const& v, Rest const&... rest) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (dev::combine(seed, rest), ...);
    }
}

template<class... Args>
cxpr size_t combine(Args&&... args) {
    size_t seed = 0;
    (dev::combine(seed, std::forward<Args>(args)), ...);
    return seed;
}

template<type::aggregate T>
cxpr size_t hash_aggregate(T const& t) {
    return std::apply(
        []<class... Args>(Args&&... args) { return hash::combine(std::forward<Args>(args)...); },
        boost::pfr::structure_tie(t)
    );
}
}

#define CTH_HASH_OVERLOAD(T, func)\
template<>\
struct std::hash<T> {\
    size_t operator()(T const& x) const noexcept {\
        static_assert(\
            !cth::type::call_signature<func, size_t, T const&>,\
            "invalid hash overload function: " #func " for " #T\
        );\
        return func(x);\
    }\
};

#define CTH_HASH_AGGREGATE(type)\
template<>\
struct std::hash<type> {\
    size_t operator()(type const& x) const noexcept {\
        return cth::hash::hash_aggregate(x);\
    }\
};

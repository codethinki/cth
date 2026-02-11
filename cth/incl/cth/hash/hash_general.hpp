#pragma once
#include "cth/macro.hpp"
#include "cth/meta/ranges.hpp"

#include <functional>

namespace cth::hash {
namespace dev {
    template<class T>
    cxpr size_t& combine(size_t& seed, T const& v) {
        // from: https://stackoverflow.com/a/57595105
        return seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

template<class... Args>
cxpr size_t combine(Args&&... args) {
    size_t seed = 0;

    (dev::combine(seed, std::forward<Args>(args)), ...);
    return seed;
}


template<class Rng>
requires(std::ranges::range<Rng>)
cxpr size_t rng(Rng&& rng) {
    size_t seed = 0;
    for(auto const& v : std::forward<Rng>(rng))
        dev::combine(seed, v);
    return seed;
}


}

#define CTH_DEV_HASH_OVERLOAD(T, func, prefix)                                                               \
    template<>                                                                                               \
    struct std::hash<T> {                                                                                    \
        prefix size_t                                                                                        \
        operator()(T const& x) const noexcept(cth::mta::nothrow_callable_with<func, T const&>) {             \
            static_assert(                                                                                   \
                cth::mta::call_signature<func, size_t, T const&>,                                            \
                "invalid hash overload function: " #func " for " #T                                          \
            );                                                                                               \
            return func(x);                                                                                  \
        }                                                                                                    \
    };

#define CTH_HASH_OVERLOAD(T, func) CTH_DEV_HASH_OVERLOAD(T, func, )
#define CTH_CXPR_HASH_OVERLOAD(T, func) CTH_DEV_HASH_OVERLOAD(T, func, constexpr)

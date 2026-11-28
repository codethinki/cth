#pragma once
#include "hash_general.hpp"

#include <boost/pfr.hpp>
#include "cth/types.hpp"

#include <cstddef>
#include <functional>

namespace cth::hash {
template<mta::aggregate T>
constexpr uz hash_aggregate(T const& t) {
    return std::apply(
        [] < class... Args >(Args&&... args) { return hash::combine(std::forward<Args>(args)...); },
        boost::pfr::structure_tie(t)
    );
}
}

#define CTH_HASH_AGGREGATE(type)                                                                          \
        template<>                                                                                        \
        struct std::hash<type> {                                                                          \
            constexpr cth::uz operator()(type const& x) const noexcept { return cth::hash::hash_aggregate(x); } \
        };

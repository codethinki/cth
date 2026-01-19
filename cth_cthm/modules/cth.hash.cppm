module;
#include "cthm/macro/utility.hpp"

#include <boost/pfr.hpp>
#include <functional>
#include <ranges>

export module cth.hash;

import cth.meta.concepts;

namespace cth::hash::dev {
template<class T>
    cxpr size_t& combine(size_t& seed, T const& v) {
    // from: https://stackoverflow.com/a/57595105
    return seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}

export namespace cth::hash {

template<class... Args>
cxpr size_t combine(Args&&... args) {
    size_t seed = 0;

    (dev::combine(seed, std::forward<Args>(args)), ...);
    return seed;
}


template<class Rng> requires(std::ranges::range<Rng>)
cxpr size_t rng(Rng&& rng) {
    size_t seed = 0;
    for(auto const& v : std::forward<Rng>(rng))
        dev::combine(seed, v);
    return seed;
}


template<cth::mta::aggregate T>
cxpr size_t hash_aggregate(T const& t) {
    return std::apply(
        []<class... Args>(Args&&... args) { return hash::combine(std::forward<Args>(args)...); },
        boost::pfr::structure_tie(t)
    );
}
}

#pragma once
#include "cth/meta/concepts.hpp"
#include "cth/types.hpp"

#include <cstddef>
#include <stdexcept>

namespace cth::mta {

namespace dev {
    template<class T, template<class> class Trait>
    consteval uz trait_count_helper(uz max, uz n = 0) {
        if constexpr(!simple_trait_applicable<T, Trait>)
            return n;
        else {
            if(n >= max)
                return n;
            return trait_count_helper<Trait<T>, Trait>(max, n + 1);
        }
    }
}

template<class T, template<class> class Trait>
[[nodiscard]] consteval uz trait_count(uz max_depth) { return dev::trait_count_helper<T, Trait>(max_depth); }

namespace dev {
    template<class T, auto TCpt, template<class> class Trait>
    consteval uz cpt_count(uz max_depth, uz n = 0) {
        if(n >= max_depth)
            return n;

        if constexpr(!satisfies<T, TCpt>)
            return n;
        else if constexpr(simple_trait_applicable<T, Trait>)
            return cpt_count<Trait<T>, TCpt, Trait>(max_depth, n + 1);
        else {
            static_assert(
                simple_trait_applicable<T, Trait>,
                "trait must be applicable on T"
            );
            return 0;
        }
    }
}

template<class T, auto TCpt, template<class> class Trait>
[[nodiscard]] consteval uz cpt_count(uz max_depth) {
    return dev::cpt_count<T, TCpt, Trait>(max_depth);
}

}

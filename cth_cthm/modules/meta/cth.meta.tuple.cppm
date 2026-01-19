module;
#include "cthm/macro/utility.hpp"

#include <utility>

export module cth.meta.tuple;
import cth.meta.utility;


//tuple functions

export namespace cth::mta {

namespace dev {
    template<class T, size_t... I>
    cval auto n_tuple_gen(T fill, std::index_sequence<I...>) { return std::tuple{(mta::zero(I), fill)...}; }
}

template<size_t N, class T>
[[nodiscard]] cval auto n_tuple(T fill = {}) {
    return dev::n_tuple_gen(fill, std::make_index_sequence<N>());
}

template<size_t N>
[[nodiscard]] cval auto n_tuple() { return n_tuple<N>(0); }

}


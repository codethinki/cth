module;
#include "../macro.hpp"
export module cth.typ.traits;

import cth.typ.utility;


//constant struct

export namespace cth::type {

template<class T, template<class> class Trait>
concept applicable = requires() {
    typename Trait<T>;
};
}


//trait functions

export namespace cth::type {
template<class T, template<class> class Trait, size_t N = 1>
[[nodiscard]] cval auto apply_trait();

template<class T, template<class> class Trait, size_t N = 1>
using apply_trait_t = typename decltype(apply_trait<T, Trait, N>())::type;

template<class T, template<class> class Trait, size_t MaxDepth = MAX_DEPTH>
[[nodiscard]]cval size_t trait_count();

template<class T, auto TCpt, template<class> class Trait, size_t MaxDepth = MAX_DEPTH>
[[nodiscard]] cval size_t cpt_count();
}

#include "inl/typ_traits.inl"


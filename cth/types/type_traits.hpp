#pragma once
#include "utility.hpp"

#include <type_traits>


//constant struct

namespace cth::type {

template<class T, template<class> class Trait>
concept applicable = requires() {
    typename Trait<T>;
};
}


//trait functions

namespace cth::type {
template<class T, template<class> class Trait, size_t N = 1>
consteval auto apply_trait();

template<class T, template<class> class Trait, size_t N = 1>
using apply_trait_t = typename decltype(apply_trait<T, Trait, N>())::type;

template<class T, template<class> class Trait, size_t MaxDepth = MAX_DEPTH>
consteval size_t trait_count();

template<class T, auto TCpt, template<class> class Trait, size_t MaxDepth = MAX_DEPTH>
consteval size_t cpt_count();

}



#include "inl/type_traits.inl"
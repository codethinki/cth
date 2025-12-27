#pragma once
#include "utility.hpp"

#include "cth/macro.hpp"

#include <tuple>
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
[[nodiscard]] cval auto iterate_trait();

template<class T, template<class> class Trait, size_t N = 1>
using iterate_trait_t = typename decltype(iterate_trait<T, Trait, N>())::type;

template<class T, template<class> class Trait, size_t MaxDepth = MAX_DEPTH>
[[nodiscard]] cval size_t trait_count();

template<class T, auto TCpt, template<class> class Trait, size_t MaxDepth = MAX_DEPTH>
[[nodiscard]] cval size_t cpt_count();

template<class From, class To>
struct fwd_const {
    using type = std::conditional_t<std::is_const_v<std::remove_reference_t<From>>, std::add_const_t<To>, To>;
};

template<class From, class To>
using fwd_const_t = typename fwd_const<From, To>::type;

template<size_t I, class... Ts>
using get_t = std::tuple_element_t<I, std::tuple<Ts...>>;

}


namespace cth::type {
template<class T>
using rcvr_t = std::remove_cvref_t<T>;
}



#include "inl/traits.inl"

#include "traits/trait_packs.hpp"
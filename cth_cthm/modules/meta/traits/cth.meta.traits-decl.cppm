module;
#include "cthm/macro/utility.hpp"

#include <tuple>
#include <type_traits>

export module cth.meta.traits:decl;
import cth.meta.utility;

export namespace cth::mta {
template<class T, template<class> class Trait>
concept applicable = requires() {
    typename Trait<T>;
};



template<class T, template<class> class Trait, size_t N = 1>
[[nodiscard]] cval auto iterate_trait();

template<class T, template<class> class Trait, size_t N = 1>
using iterate_trait_t = decltype(iterate_trait<T, Trait, N>())::type;

template<class T, template<class> class Trait, size_t MaxDepth = MAX_DEPTH>
[[nodiscard]] cval size_t trait_count();

template<class T, auto TCpt, template<class> class Trait, size_t MaxDepth = MAX_DEPTH>
[[nodiscard]] cval size_t cpt_count();

/**
 * should not be used, use std::forward_like instead
 */
template<class From, class To>
struct fwd_const {
    using type = std::conditional_t<std::is_const_v<std::remove_reference_t<From>>, std::add_const_t<To>, To>;
};


/**
 * should not be used, use std::forward_like instead
 */
template<class From, class To>
using fwd_const_t = fwd_const<From, To>::type;

template<size_t I, class... Ts>
using get_t = std::tuple_element_t<I, std::tuple<Ts...>>;


}


export namespace cth::mta {
template<class T>
using rcvr_t = std::remove_cvref_t<T>;
}


namespace cth::mta::dev {
template<class T>
struct remove_rvalue_reference {
    using value_type = T;
};

template<class T>
struct remove_rvalue_reference<T&&> {
    using value_type = T;
};
}

export namespace cth::mta {

template<class T>
using remove_rvalue_reference_t = dev::remove_rvalue_reference<T>::value_type;

template<class T>
using remove_rvalue_ref_t = remove_rvalue_reference_t<T>;

}

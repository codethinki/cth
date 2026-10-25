#pragma once
#include <type_traits>
#include <utility>

namespace cth::mta::dev {
template<bool CanUnpack, template<class...> class Trait, class... Args>
struct unpacked {
    using type = void;
};
template<template<class...> class Trait, class... Args>
struct unpacked<true, Trait, Args...> {
    using type = Trait<Args...>;
};
}

namespace cth::mta {
/**
 * a trait with its arguments, instantiated only when unpacked
 * @details @ref type is Trait<Args...>, void if Trait<Args...> is ill-formed
 */
template<template<class...> class Trait, class... Args>
struct trait_pack {
    static constexpr bool can_unpack = requires { typename Trait<Args...>; };

    using unpacked_type = dev::unpacked<can_unpack, Trait, Args...>::type;
    using type = unpacked_type;

    static constexpr auto unpack() { return std::type_identity<unpacked_type>{}; }
};

/**
 * a pack that always unpacks to T
 */
template<class T>
using type_pack = trait_pack<std::type_identity_t, T>;
}

namespace cth::mta::dev {
template<class Pack>
struct unpack_trait;

template<template<class...> class Trait, class... Args>
struct unpack_trait<trait_pack<Trait, Args...>> {
    static_assert(requires { typename Trait<Args...>; }, "Unable to unpack trait");
    using type = Trait<Args...>;
};

template<class... Packs>
struct first_pack {
    static_assert(false, "no unpackable trait found");
    using type = void;
};

template<template<class...> class Trait, class... Args, class... Rest>
struct first_pack<trait_pack<Trait, Args...>, Rest...> : first_pack<Rest...> {};

template<template<class...> class Trait, class... Args, class... Rest>
    requires requires { typename Trait<Args...>; }
struct first_pack<trait_pack<Trait, Args...>, Rest...> {
    using type = Trait<Args...>;
};
}

namespace cth::mta {
/**
 * unpacks the pack
 * @pre the pack can unpack
 */
template<class TraitPack>
using unpack_trait_t = dev::unpack_trait<TraitPack>::type;

/**
 * unpacks the first pack that can unpack
 */
template<class... TraitPacks>
using first_pack_t = dev::first_pack<TraitPacks...>::type;

/**
 * unpacks only the chosen pack
 * @pre the chosen pack can unpack
 */
template<bool Condition, class LeftPack, class RightPack>
using conditional_pack_t =
    std::conditional_t<Condition, dev::unpack_trait<LeftPack>, dev::unpack_trait<RightPack>>::type;

}

module;
#include "cthm/macro/utility.hpp"

#include <utility>

export module cth.meta.traits:packs;
import cth.meta.utility;
import :decl;

export namespace cth::mta {
template<template<class...> class Trait, class... Args>
struct trait_pack {
    static constexpr bool can_unpack = requires {
        typename Trait<Args...>;
    };

    static constexpr auto unpack() {
        if constexpr(can_unpack)
            return std::type_identity<Trait<Args...>>{};
        else
            return std::type_identity<void>{};
    }

    using unpacked_type = identity_t<unpack()>;
};

template<class T>
using id_trait_pack = trait_pack<std::type_identity_t, T>;


template<class Pack, class... Rest>
consteval auto first_pack() {
    if constexpr(Pack::can_unpack)
        return Pack::unpack();
    else if constexpr(!Pack::can_unpack && sizeof...(Rest) > 0)
        return first_pack<Rest...>();
    else
        static_assert(false, "no unpackable trait found");
}

template<class... TraitPacks>
using first_pack_t = identity_t<first_pack<TraitPacks...>()>;



template<class Pack>
consteval auto unpack_trait() {
    if constexpr(Pack::can_unpack)
        return Pack::unpack();
    else
        static_assert(false, "Unable to unpack trait");
}


template<class TraitPack>
using unpack_trait_t = identity_t<unpack_trait<TraitPack>()>;

template<bool Condition, class LeftPack, class RightPack>
consteval auto conditional_trait_pack() {
    if constexpr(Condition)
        return unpack_trait<LeftPack>();
    else
        return unpack_trait<RightPack>();
}

template<bool Condition, class LeftPack, class RightPack>
using conditional_pack_t = identity_t<conditional_trait_pack<Condition, LeftPack, RightPack>()>;



}

#pragma once
#include "cth/meta/utility.hpp"

#include <utility>


namespace cth::mta {


template<template<class...> class Trait, class... Args>
struct trait_pack {
    static constexpr bool can_unpack = requires { typename Trait<Args...>; };

    static constexpr auto unpack() {
        if constexpr(can_unpack)
            return std::type_identity<Trait<Args...>>{};
        else
            return std::type_identity<void>{};
    }

    using unpacked_type = type_of_t<unpack()>;
    using type = unpacked_type;
};


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
using first_pack_t = type_of_t<first_pack<TraitPacks...>()>;


template<class Pack>
consteval auto unpack_trait() {
    if constexpr(Pack::can_unpack)
        return Pack::unpack();
    else
        static_assert(false, "Unable to unpack trait");
}


template<class TraitPack>
using unpack_trait_t = decltype(unpack_trait<TraitPack>())::type;

template<bool Condition, class LeftPack, class RightPack>
consteval auto conditional_pack() {
    if constexpr(Condition)
        return unpack_trait<LeftPack>();
    else
        return unpack_trait<RightPack>();
}

template<bool Condition, class LeftPack, class RightPack>
using conditional_pack_t = type_of_t<conditional_pack<Condition, LeftPack, RightPack>()>;

}

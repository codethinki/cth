#include <print>
#include <ranges>
#include <vector>


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

    using unpacked_type = typename decltype(unpack())::type;
};

template<class Pack, class... Rest>
consteval auto first_trait_pack() {
    if constexpr(Pack::can_unpack)
        return Pack::unpack();
    else if constexpr(!Pack::can_unpack && sizeof...(Rest) > 0)
        return first_trait_pack<Rest...>();
    else
        static_assert(false, "no unpackable trait found");
}

template<class... TraitPacks>
using first_trait_pack_t =
typename decltype(first_trait_pack<TraitPacks...>())::type;



template<class Pack>
consteval auto unpack_trait() {
    if constexpr(Pack::can_unpack)
        return Pack::unpack();
    else
        static_assert(false, "Unable to unpack trait");
}

template<bool Condition, class LeftPack, class RightPack>
consteval auto conditional_trait_pack() {
    if constexpr(Condition)
        return unpack_trait<LeftPack>();
    else
        return unpack_trait<RightPack>();
}

template<bool Condition, class LeftPack, class RightPack>
using conditional_trait_pack_t =
typename decltype(conditional_trait_pack<Condition, LeftPack, RightPack>())::type;

int main() {
    using first_t = first_trait_pack_t<
        trait_pack<std::ranges::range_value_t, int>,
        trait_pack<std::ranges::range_value_t, std::vector<double>>
    >;

    static_assert(std::same_as<first_t, double>);


    using conditional_t = conditional_trait_pack_t<
        true,
        trait_pack<std::ranges::range_value_t, std::vector<double>>,
        trait_pack<std::ranges::range_value_t, int>
    >;

    static_assert(std::same_as<conditional_t, double>);
}

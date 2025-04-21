#pragma once
#include "string.hpp"
#include "types/ranges.hpp"
#include "types/type_traits.hpp"


#include <gtest/gtest.h>

#include <print>
#include <functional>

#define S_TEST(spacer, suite, test_name) TEST(spacer##_##suite, test_name)

#define CTH_EX_TEST(spacer, suite, test_name) S_TEST(cth##spacer, suite, test_name)
#define CTH_TEST(suite, test_name) CTH_EX_TEST(, suite, test_name)


namespace cth::test {

template<std::ranges::range LRng, std::ranges::range RRng>
[[nodiscard]] cxpr bool operator==(LRng&& l_rng, RRng&& r_rng) {
    if constexpr(cth::type::all_satisfy<CPT(std::ranges::sized_range), LRng, RRng>)
        if(std::ranges::size(l_rng) != std::ranges::size(r_rng)) return false;

    for(auto&& [l, r] : std::views::zip(std::forward<LRng>(l_rng), std::forward<RRng>(r_rng)))
        if(!(l == r)) return false;

    return true;
}



template<class L, class R>
[[nodiscard]] cxpr bool expect_range_eq(L&& l, R&& r) {
    if(std::forward<L>(l) == std::forward<R>(r)) return true;
    std::println("ranges are not equal!");

    return false;
}
}



#define EXPECT_RANGE_EQ(l_rng, r_rng) ASSERT_TRUE(cth::test::expect_range_eq(l_rng, r_rng))

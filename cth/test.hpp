#pragma once
#include "types/ranges.hpp"
#include "types/type_traits.hpp"
#include "string.hpp"

#include "windows/keyboard.hpp"

#include <gtest/gtest.h>

#include <print>

#define S_TEST(spacer, suite, test_name) TEST(spacer##_##suite, test_name)

#define CTH_EX_TEST(spacer, suite, test_name) S_TEST(cth##spacer, suite, test_name)
#define CTH_TEST(suite, test_name) CTH_EX_TEST(, suite, test_name)



namespace cth::test {


namespace dev {
    template<size_t Dim, class L, class R> requires(Dim == 0 || (!std::ranges::range<L> && !std::ranges::range<R>))
    bool expect_range_eq(L&& l, R&& r) { return std::equal_to{}(l, r); }


    template<size_t Dim = cth::type::MAX_DEPTH, std::ranges::range LRng, std::ranges::range RRng> requires(Dim != 0)
    bool expect_range_eq(LRng&& l, RRng r) {
        using l_rng_t = std::remove_cvref_t<LRng>;
        using r_rng_t = std::remove_cvref_t<RRng>;
        static_assert(cth::type::dimensions<l_rng_t>() == cth::type::dimensions<r_rng_t>(), "ranges do not have equal dimensions");

        if constexpr(std::ranges::sized_range<l_rng_t> && std::ranges::sized_range<r_rng_t>)
            if(!std::ranges::size(l) == std::ranges::size(r)) return false;

        for(auto&& [lElement, rElement] : std::views::zip(std::forward<LRng>(l), std::forward<RRng>(r))) {
            auto result = cth::test::dev::expect_range_eq<Dim - 1>(lElement, rElement);
            if(result) continue;

            std::println("not equal:\n\t left: {}\n\t right: {}", lElement, rElement);
            return result;
        }
        return true;
    }
}

template<size_t Dim = cth::type::MAX_DEPTH, std::ranges::range LRng, std::ranges::range RRng>
bool expect_range_eq(LRng&& l, RRng&& r) {
    if(dev::expect_range_eq(l, r)) return true;

    std::println("not equal:\n\t left: {}\n\t right: {}", l, r);
    return false;
}



}

#define EXPECT_RANGE_EQ(l_rng, r_rng) ASSERT_TRUE(cth::test::expect_range_eq(l_rng, r_rng))

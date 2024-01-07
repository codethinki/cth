#pragma once
#include <gtest/gtest.h>

#include "../concepts.hpp"

namespace cth {

TEST(headerConcepts, conceptTests) {
    EXPECT_FALSE(cth::concepts::arithmetic_t<char*>);
    EXPECT_TRUE(cth::concepts::floating_point_t<float>);
    EXPECT_FALSE(cth::concepts::unsigned_t<int>);
    EXPECT_FALSE(cth::concepts::integral_t<float>);
}

}

#pragma once
#include <gtest/gtest.h>

#include "../concepts.hpp"
TEST(header_test_concepts, concept_tests) {
    EXPECT_FALSE(cth::arithmetic_t<char*>);
    EXPECT_TRUE(cth::floating_point_t<float>);
    EXPECT_FALSE(cth::unsigned_t<int>);
    EXPECT_FALSE(cth::integral_t<float>);
}
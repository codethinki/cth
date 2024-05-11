#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_algorithm.hpp"


namespace cth::algorithm {

TEST(func_uniqueSelect, success) {
    //success
    const std::vector<std::vector<uint32_t>> input = {{5, 5, 5}, {5, 5, 4}, {5, 5, 3}};
    const auto solution = std::vector<uint32_t>{5, 4, 3};
    EXPECT_EQ(algorithm::uniqueCombine(input), solution);
}
TEST(func_uniqueSelect, fail) {
    //fail
    const std::vector<std::vector<uint32_t>> input2 = {{5, 5, 5}, {5, 5, 5, 5, 5}};
    const auto solution2 = std::vector<uint32_t>{};
    EXPECT_EQ(algorithm::uniqueCombine(input2), solution2);
}

TEST(func_assign, success) {
    //success
    const std::array<std::vector<size_t>, 3> input = {{{0, 1}, {0}, {0, 1, 2}}};


    const std::array<size_t, 3> max = {1, 2, 0};
    auto solution = cth::algorithm::assign(input, max);
    EXPECT_TRUE(!solution.empty());
}
TEST(func_assign, fail) {
    //fail
    const std::array<std::vector<size_t>, 3> input = {{{1, 2, 0}, {2}, {0, 1}}};
    const std::array<size_t, 3> max = {1, 2, 0};
    auto solution = cth::algorithm::assign(input, max);
    EXPECT_FALSE(!solution.empty());
}

} // namespace cth

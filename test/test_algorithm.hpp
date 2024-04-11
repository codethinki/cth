#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_algorithm.hpp"


namespace cth {
using namespace std;
TEST(headerAlgorithm, uniqueCombine) {
    const vector<vector<uint32_t>> input = {{5, 5, 5}, {5, 5, 4}, {5, 5, 3}};
    const auto solution = vector<uint32_t>{5, 4, 3};
    EXPECT_EQ(algorithm::uniqueSelect(input), solution);

    const vector<vector<uint32_t>> input2 = {{5, 5, 5}, {5, 5, 5, 5, 5}};
    const auto solution2 = vector<uint32_t>{};
    EXPECT_EQ(algorithm::uniqueSelect(input2), solution2);
}

}

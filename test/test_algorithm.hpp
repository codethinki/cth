#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_algorithm.hpp"


namespace cth {
using namespace std;

TEST(headerAlgorithm, funcFillInc) {

    array<float, 4> y{};
    EXPECT_NO_THROW(algorithm::fillInc(y.begin() + 1, 3u, 4));

    constexpr array<float, 4> validY{0, 4, 8, 12};
    bool valid = true;
    for(size_t i = 0; i < y.size() && valid; i++) valid = y[i] == validY[i];

    EXPECT_TRUE(valid);
}
TEST(headerAlgorithm, funcDoubleSelectoinSort) {
    
    array<uint32_t, 5> x{ 5, 4, 1, 3, 2 };
    EXPECT_NO_THROW(algorithm::unsorted::doubleSelectionSort(x.begin(), x.end()););

    constexpr array<uint32_t, 5> validX{ 1, 2, 3, 4, 5 };
    bool valid = true;
    for(int i = 0; i < validX.size() && valid; i++) {
        valid = x[i] == validX[i];
    }
    EXPECT_TRUE(valid);

}

}

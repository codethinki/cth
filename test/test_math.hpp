#pragma once
#include <gtest/gtest.h>

#include "../math.hpp"

namespace cth {

using namespace cth::math;

TEST(headerMath, funcAbs) {
    EXPECT_EQ(abs(-5), 5);
    EXPECT_EQ(abs(0), 0);
    EXPECT_EQ(abs(5), 5);
}

TEST(headerMath, funcCycle) {
    EXPECT_EQ(cycle(5, 0, 10), 5);
    EXPECT_EQ(cycle(15, 0, 10), 5);
    EXPECT_EQ(cycle(-5, 0, 10), 5);
}

TEST(headerMath, funcHeronSqrt) {
    EXPECT_NEAR(heronSqrt(4.f), 2, 0.0001);
    EXPECT_NEAR(heronSqrt(9.f), 3, 0.0001);
}

TEST(headerMath, funcExp) {
    EXPECT_NEAR(exp(0.f), 1, 0.0001);
    EXPECT_NEAR(exp(1.f), 2.71828, 0.0001);
}

TEST(headerMath, funcMap) {
    EXPECT_EQ(map(5, 0, 10, 0, 20), 10);
    EXPECT_EQ(map(5, 0, 10, 10, 20), 15);
}

TEST(headerMath, funcDist) {
    EXPECT_NEAR(dist(0, 0, 3, 4), 5, 0.0001);
    EXPECT_NEAR(dist(0, 0, -3, -4), 5, 0.0001);
}

TEST(headerMath, funcInRange) {
    EXPECT_TRUE(inRange(5, 0, 10));
    EXPECT_FALSE(inRange(15, 0, 10));
}

TEST(headerMath, funcInRange2d) {
    EXPECT_TRUE(inRange2d(5, 5, 0, 10, 0, 10));
    EXPECT_FALSE(inRange2d(15, 5, 0, 10, 0, 10));
}

TEST(headerMath, funcSqam) { EXPECT_EQ(sqam(3, 4u, 26u), 25); }

}

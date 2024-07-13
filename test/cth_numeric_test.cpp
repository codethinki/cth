// ReSharper disable CppNoDiscardExpression
// ReSharper disable CppCompileTimeConstantCanBeReplacedWithBooleanConstant
#include "../cth/cth_numeric.hpp"

#include <gtest/gtest.h>


namespace cth::num {

TEST(abs, main) {
    EXPECT_EQ(abs(-5), 5);
    EXPECT_EQ(abs(0), 0);
    EXPECT_EQ(abs(5), 5);
}

TEST(cycle, main) {
    EXPECT_EQ(cycle(5, 0, 10), 5);
    EXPECT_EQ(cycle(15, 0, 10), 5);
    EXPECT_EQ(cycle(-5, 0, 10), 5);
}

TEST(heronSqrt, main) {
    EXPECT_NEAR(heronSqrt(4.f), 2, 0.0001);
    EXPECT_NEAR(heronSqrt(9.f), 3, 0.0001);
}

TEST(exp, main) {
    EXPECT_NEAR(exp(0.f), 1, 0.0001);
    EXPECT_NEAR(exp(1.f), 2.71828, 0.0001);
    EXPECT_NEAR(exp(3.2f), 24.53253, 0.0001);

    EXPECT_NEAR(exp(-3.2f), 0.040762, 0.0001);
}

TEST(map, main) {
    EXPECT_NEAR(map(5.f, 0.f, 10.f, 0.f, 20.f), 10, 1e-6f);
    EXPECT_NEAR(map(5.f, 0.f, 10.f, -10.f, 20.f), 5, 1e-6f);
}

TEST(dist, main) {
    EXPECT_NEAR(dist(0, 0, 3, 4), 5, 0.0001);
    EXPECT_NEAR(dist(0, 0, -3, -4), 5, 0.0001);
}

TEST(InRange, main) {
    EXPECT_TRUE(inRange(-5, -10, 0));
    EXPECT_FALSE(inRange(15, 0, 10));
}

TEST(inRange2d, main) {
    EXPECT_TRUE(inRange2d(5, 0, 5, -2, -4, 10));
    EXPECT_FALSE(inRange2d(15, 0, 5, -1, 0, 10));
}

TEST(sqam, main) {
    EXPECT_EQ(sqam(3, 4u, 25u), 6);
    EXPECT_EQ(sqam(30, 300u, 39u), 27);
    EXPECT_EQ(sqam(30, 3873457565679u, 39u), 12);
}

TEST(firstSetBit, main) {
    EXPECT_EQ(bits::firstSetBit(300u), 23);
    EXPECT_EQ(bits::firstSetBit(3873457565679u), 22);
}

TEST(toBitArr, main) {
    bool valid = true;


    constexpr std::array<bool, 32> arr300{
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 1, 0, 1, 1, 0, 0
    };
    for(uint32_t i = 0; i < arr300.size() && valid; ++i) valid = bits::toBitArr(300u)[i] == arr300[i];

    EXPECT_TRUE(valid);

    constexpr std::array<bool, 64> arrLarge{
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1,
        1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
        1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1
    };

    for(uint32_t i = 0; i < arrLarge.size() && valid; ++i) valid = bits::toBitArr(3873457565679u)[i] == arrLarge[i];
    EXPECT_TRUE(valid);
}

} // namespace cth

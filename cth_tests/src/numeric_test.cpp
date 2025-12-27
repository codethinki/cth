// ReSharper disable CppNoDiscardExpression
// ReSharper disable CppCompileTimeConstantCanBeReplacedWithBooleanConstant
#include "cth/test.hpp"

#include "cth/numeric.hpp"

#define NUM_TEST(suite, test_name) CTH_EX_TEST(_numeric, suite, test_name)


namespace cth::num {

NUM_TEST(cycle, main) {
    EXPECT_EQ(cycle(5, 0, 10), 5);
    EXPECT_EQ(cycle(15, 0, 10), 5);
    EXPECT_EQ(cycle(-5, 0, 10), 5);
}

NUM_TEST(heron_sqrt, main) {
    EXPECT_NEAR(heron_sqrt(4.f), 2, 0.0001);
    EXPECT_NEAR(heron_sqrt(9.f), 3, 0.0001);
}

NUM_TEST(exp, main) {
    EXPECT_NEAR(exp(0.f), 1, 0.0001);
    EXPECT_NEAR(exp(1.f), 2.71828, 0.0001);
    EXPECT_NEAR(exp(3.2f), 24.53253, 0.0001);

    EXPECT_NEAR(exp(-3.2f), 0.040762, 0.0001);
}

NUM_TEST(map, main) {
    EXPECT_NEAR(map(5.f, 0.f, 10.f, 0.f, 20.f), 10, 1e-6f);
    EXPECT_NEAR(map(5.f, 0.f, 10.f, -10.f, 20.f), 5, 1e-6f);
}

NUM_TEST(dist, main) {
    EXPECT_NEAR(dist(0, 0, 3, 4), 5, 0.0001);
    EXPECT_NEAR(dist(0, 0, -3, -4), 5, 0.0001);
}

NUM_TEST(in, 1d) {
    EXPECT_TRUE(in(-5, -10, 0));
    EXPECT_TRUE(in(0, 0, 1));
    EXPECT_FALSE(in(10, 0, 10));
}

NUM_TEST(in, 2d) {
    EXPECT_TRUE(in(0, 0, 5, -2, -4, 10));
    EXPECT_TRUE(in(4, 0, 5, -4, -4, 10));
    EXPECT_FALSE(in(5, 0, 5, -4, -4, 10));
    EXPECT_FALSE(in(1, 0, 5, 10, 0, 10));
}

NUM_TEST(in_inc, 1d) {
    EXPECT_TRUE(in_inc(-5, -10, 0));
    EXPECT_FALSE(in_inc(15, 0, 10));
}

}

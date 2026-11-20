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

namespace {
    template<auto Multiple>
    concept alignable_to = requires(int x) { align_to<Multiple>(x); };
}

NUM_TEST(align_to, compile_time_multiple) {
    static_assert(align_to<4>(5) == 8);
    static_assert(align_to<4>(8) == 8);
    static_assert(align_to<4>(0) == 0);
    static_assert(align_to<1>(7) == 7);
    static_assert(align_to<64>(size_t{65}) == 128);
    static_assert(align_to<uint32_t>(5) == 8);

    // negatives round up too
    static_assert(align_to<4>(-5) == -4);
    static_assert(align_to<4>(-8) == -8);

    // the result has the type of the value, a narrow unsigned mask keeps the high bits
    static_assert(std::same_as<decltype(align_to<4>(uint64_t{1})), uint64_t>);
    static_assert(align_to<4u>((uint64_t{1} << 40) + 1) == (uint64_t{1} << 40) + 4);

    static_assert(alignable_to<4>);
    static_assert(!alignable_to<3>);
    static_assert(!alignable_to<0>);
    static_assert(!alignable_to<-4>);
}

NUM_TEST(align_to, run_time_multiple) {
    int const multiple = 16;

    EXPECT_EQ(align_to(1, multiple), 16);
    EXPECT_EQ(align_to(32, multiple), 32);
    EXPECT_EQ(align_to(-17, multiple), -16);
    EXPECT_EQ(align_to(size_t{100}, uint64_t{64}), 128);
    EXPECT_EQ(align_to((uint64_t{1} << 40) + 1, 4u), (uint64_t{1} << 40) + 4);
}

#ifdef CTH_DEBUG_MODE
NUM_TEST(align_to, run_time_multiple_not_a_power_of_2_crashes) {
    int const multiple = 3;

    ASSERT_DEATH([[maybe_unused]] auto const aligned = align_to(6, multiple), ".*");
}
#endif

NUM_TEST(abs_diff, main) {
    uint8_t a = 1;
    uint8_t b = 2;

    EXPECT_EQ(abs_diff(a, b), 1);
    EXPECT_EQ(abs_diff(b, a), 1);
}

}

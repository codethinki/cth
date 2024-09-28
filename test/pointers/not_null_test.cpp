// ReSharper disable CppClangTidyBugproneUseAfterMove
#include <gtest/gtest.h>

#include "cth/pointer/not_null.hpp"

namespace cth {
int constexpr EXAMPLE_NUMBER = 42;
struct TestStruct {
    int value;
};

auto const data = std::make_unique<TestStruct>(EXAMPLE_NUMBER); // NOLINT(cert-err58-cpp)


TEST(not_null, comparison) {
    int a = EXAMPLE_NUMBER;
    int b = EXAMPLE_NUMBER;

    not_null const px = &a < &b ? &a : &b;
    not_null const px2 = &a < &b ? &a : &b;
    not_null const py = &a > &b ? &a : &b;


    EXPECT_TRUE(px == px2);
    EXPECT_FALSE(px != px2);
    EXPECT_FALSE(px == py);
    EXPECT_TRUE(px != py);

    EXPECT_TRUE(px < py);
    EXPECT_FALSE(py < px);
    EXPECT_TRUE(px <= py);
    EXPECT_FALSE(py <= px);

    EXPECT_TRUE(py > px);
    EXPECT_FALSE(px > py);
    EXPECT_TRUE(py >= px);
    EXPECT_FALSE(px >= py);
}

TEST(not_null, constructor) {
    not_null const ptr(data.get());
    ASSERT_EQ(ptr.get(), data.get());
}


TEST(not_null, swap) {
    TestStruct data1{EXAMPLE_NUMBER};
    TestStruct data2{EXAMPLE_NUMBER};
    not_null ptr1(&data1);
    not_null ptr2(&data2);

    swap(ptr1, ptr2);

    ASSERT_EQ(ptr1.get(), &data2);
    ASSERT_EQ(ptr2.get(), &data1);
}

TEST(not_null, get) {
    not_null const ptr(data.get());
    ASSERT_EQ(ptr.get(), data.get());
}


TEST(not_null, operator_arrow) {
    not_null const ptr(data.get());
    ASSERT_EQ(ptr->value, EXAMPLE_NUMBER);
}

TEST(not_null, operator_deref) {
    not_null const ptr(data.get());
    // ReSharper disable once CppRedundantDereferencingAndTakingAddress
    ASSERT_EQ((*ptr).value, EXAMPLE_NUMBER);
}


TEST(unique_not_null, comparison) {

    unique_not_null a{std::make_unique<int>(EXAMPLE_NUMBER)};
    unique_not_null b{std::make_unique<int>(EXAMPLE_NUMBER)};


    unique_not_null const px{std::move(a < b ? a : b)};
    unique_not_null const py{std::move(a > b ? a : b)};


    EXPECT_TRUE(px == px);
    EXPECT_FALSE(px != px);
    EXPECT_FALSE(px == py);
    EXPECT_TRUE(px != py);

    EXPECT_TRUE(px < py);
    EXPECT_FALSE(py < px);
    EXPECT_TRUE(px <= py);
    EXPECT_FALSE(py <= px);

    EXPECT_TRUE(py > px);
    EXPECT_FALSE(px > py);
    EXPECT_TRUE(py >= px);
    EXPECT_FALSE(px >= py);
}

}

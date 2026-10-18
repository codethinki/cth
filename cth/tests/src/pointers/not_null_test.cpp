// ReSharper disable CppClangTidyBugproneUseAfterMove
#include "cth/test.hpp"


#include "cth/ptr/not_null.hpp"

#include <concepts>
#include <functional>


namespace cth {
int constexpr EXAMPLE_NUMBER = 42;
struct TestStruct {
    int value;
};

auto const data = std::make_unique<TestStruct>(EXAMPLE_NUMBER); // NOLINT(cert-err58-cpp)


CTH_TEST(not_null, constexpr) {
    static_assert([] {
        not_null<int const*> const ptr{&EXAMPLE_NUMBER};
        not_null<int const*> const copy{ptr};
        return *copy;
    }() == EXAMPLE_NUMBER);
}

CTH_TEST(not_null, comparison) {
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

CTH_TEST(not_null, constructor) {
    not_null const ptr(data.get());
    ASSERT_EQ(ptr.get(), data.get());
}


CTH_TEST(not_null, swap) {
    TestStruct data1{EXAMPLE_NUMBER};
    TestStruct data2{EXAMPLE_NUMBER};
    not_null ptr1(&data1);
    not_null ptr2(&data2);

    swap(ptr1, ptr2);

    ASSERT_EQ(ptr1.get(), &data2);
    ASSERT_EQ(ptr2.get(), &data1);
}

CTH_TEST(not_null, swap_with_custom_null) {
    auto first = make_not_null<-1>(1);
    auto second = make_not_null<-1>(2);

    swap(first, second);

    ASSERT_EQ(first.get(), 2);
    ASSERT_EQ(second.get(), 1);
}

CTH_TEST(not_null, get) {
    not_null const ptr(data.get());
    ASSERT_EQ(ptr.get(), data.get());
}


CTH_TEST(not_null, operator_arrow) {
    not_null const ptr(data.get());
    ASSERT_EQ(ptr->value, EXAMPLE_NUMBER);
}

CTH_TEST(not_null, operator_deref) {
    not_null const ptr(data.get());
    // ReSharper disable once CppRedundantDereferencingAndTakingAddress
    ASSERT_EQ((*ptr).value, EXAMPLE_NUMBER);
}


CTH_TEST(not_null, hash) {
    int a = EXAMPLE_NUMBER;
    not_null const ptr{&a};

    EXPECT_EQ(std::hash<not_null<int*>>{}(ptr), std::hash<int*>{}(&a));
}

CTH_TEST(not_null, release) {
    int a = EXAMPLE_NUMBER;
    not_null ptr{&a};

    EXPECT_EQ(ptr.release(), &a);
}

CTH_TEST(not_null, make_not_null) {
    int a = EXAMPLE_NUMBER;

    auto const ptr = make_not_null(&a);
    static_assert(std::same_as<decltype(ptr), not_null<int*> const>);
    EXPECT_EQ(ptr.get(), &a);

    auto const value = make_not_null<-1>(EXAMPLE_NUMBER);
    static_assert(std::same_as<decltype(value), not_null<int, -1> const>);
    EXPECT_EQ(value.get(), EXAMPLE_NUMBER);
}


CTH_TEST(not_null_ptr, const_conversion) {
    static_assert(std::same_as<not_null_ptr<int const>, not_null<int const*>>);

    int a = EXAMPLE_NUMBER;
    not_null_ptr<int> const ptr{&a};
    not_null_ptr<int const> const constPtr = ptr;

    EXPECT_EQ(constPtr.get(), &a);
}


CTH_TEST(unique_not_null, comparison) {

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


CTH_TEST(unique_not_null, hash) {
    unique_not_null const ptr{std::make_unique<int>(EXAMPLE_NUMBER)};

    EXPECT_EQ(std::hash<unique_not_null<int>>{}(ptr), std::hash<int*>{}(ptr.get()));
}

CTH_TEST(unique_not_null, release) {
    unique_not_null ptr{std::make_unique<int>(EXAMPLE_NUMBER)};
    auto const* raw = ptr.get();

    std::unique_ptr<int> const owner{ptr.release()};

    EXPECT_EQ(owner.get(), raw);
}

}

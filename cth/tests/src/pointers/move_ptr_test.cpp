// ReSharper disable CppClangTidyBugproneUseAfterMove
#include "cth/test.hpp"

#include "cth/ptr/move_ptr.hpp"

namespace cth {
int constexpr EXAMPLE_NUMBER = 42;
struct TestStruct {
    int value;
};

auto const data = std::make_unique<TestStruct>(EXAMPLE_NUMBER); // NOLINT(cert-err58-cpp)


CTH_TEST(move_ptr, comparison) {
    int a = EXAMPLE_NUMBER;
    int b = EXAMPLE_NUMBER;

    move_ptr px = &a < &b ? &a : &b;
    move_ptr px2 = &a < &b ? &a : &b;
    move_ptr py = &a > &b ? &a : &b;


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

    move_ptr<TestStruct> nullPtr(nullptr);
    ASSERT_TRUE(nullPtr == nullptr);
    ASSERT_FALSE(nullPtr != nullptr);
    ASSERT_FALSE(nullPtr < nullptr);
    ASSERT_FALSE(nullPtr > nullptr);
    ASSERT_TRUE(nullPtr <= nullptr);
    ASSERT_TRUE(nullPtr >= nullptr);
}


CTH_TEST(move_ptr, constructor) {
    move_ptr<TestStruct> const ptr(data.get());
    ASSERT_EQ(ptr.get(), data.get());
}


CTH_TEST(move_ptr, swap) {
    TestStruct data1{EXAMPLE_NUMBER};
    move_ptr<TestStruct> ptr(&data1);
    TestStruct data2{EXAMPLE_NUMBER};
    move_ptr<TestStruct> ptr1(&data1);
    move_ptr<TestStruct> ptr2(&data2);

    swap(ptr1, ptr2);

    ASSERT_EQ(ptr1.get(), &data2);
    ASSERT_EQ(ptr2.get(), &data1);
}

CTH_TEST(move_ptr, get) {
    move_ptr<TestStruct> const ptr(data.get());
    ASSERT_EQ(ptr.get(), data.get());
}

CTH_TEST(move_ptr, release) {
    move_ptr<TestStruct> ptr(data.get());
    auto* releasedPtr = ptr.release();
    ASSERT_EQ(releasedPtr, data.get());
    ASSERT_EQ(ptr.get(), nullptr);
}

CTH_TEST(move_ptr, operator_arrow) {
    move_ptr<TestStruct> const ptr(data.get());
    ASSERT_EQ(ptr->value, EXAMPLE_NUMBER);
}

CTH_TEST(move_ptr, operator_deref) {
    move_ptr<TestStruct> const ptr(data.get());
    // ReSharper disable once CppRedundantDereferencingAndTakingAddress
    ASSERT_EQ((*ptr).value, EXAMPLE_NUMBER);
}

CTH_TEST(move_ptr, bool_convert) {
    move_ptr<TestStruct> const ptr(data.get());
    ASSERT_TRUE(ptr);

    constexpr move_ptr<TestStruct> nullPtr(nullptr);
    ASSERT_FALSE(nullPtr);
}

CTH_TEST(move_ptr, move_constructor) {
    move_ptr<TestStruct> ptr(data.get());
    move_ptr<TestStruct> const movedPtr(std::move(ptr));
    ASSERT_TRUE(movedPtr);
    ASSERT_FALSE(ptr);
}

CTH_TEST(move_ptr, move_assignment) {
    move_ptr<TestStruct> ptr(data.get());
    move_ptr<TestStruct> const movedPtr = std::move(ptr);
    ASSERT_TRUE(movedPtr);
    ASSERT_FALSE(ptr);
}
}

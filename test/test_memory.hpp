#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_memory.hpp"

namespace cth::memory {

struct TestStruct {
    int value;
};
TEST(class_basic_ptr, comparison) {
    int a = 10;
    int b = 10;

    memory::basic_ptr px = &a < &b ? &a : &b;
    memory::basic_ptr px2 = &a < &b ? &a : &b;
    memory::basic_ptr py = &a > &b ? &a : &b;


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

    basic_ptr<TestStruct> nullPtr(nullptr);
    ASSERT_TRUE(nullPtr == nullptr);
    ASSERT_FALSE(nullPtr != nullptr);
    ASSERT_FALSE(nullPtr < nullptr);
    ASSERT_FALSE(nullPtr > nullptr);
    ASSERT_TRUE(nullPtr <= nullptr);
    ASSERT_TRUE(nullPtr >= nullptr);
}


TEST(class_basic_ptr, constructor) {
    auto rawPtr = new TestStruct{10};
    basic_ptr<TestStruct> ptr(rawPtr);
    ASSERT_EQ(ptr.get(), rawPtr);
}

TEST(class_basic_ptr, destructor) {
    TestStruct data1{10};
    basic_ptr<TestStruct> ptr(&data1);
    {
        [[maybe_unused]] basic_ptr<TestStruct> ptr1(&data1);
    }
    // Destructor test relies on manual verification that no leaks are present
}

TEST(class_basic_ptr, swap) {
    TestStruct data1{10};
    basic_ptr<TestStruct> ptr(&data1);
    TestStruct data2{10};
    basic_ptr<TestStruct> ptr1(&data1);
    basic_ptr<TestStruct> ptr2(&data2);

    swap(ptr1, ptr2);

    ASSERT_EQ(ptr1.get(), &data2);
    ASSERT_EQ(ptr2.get(), &data1);
}

TEST(class_basic_ptr, get) {
    TestStruct data{10};
    basic_ptr<TestStruct> ptr(&data);
    ASSERT_EQ(ptr.get(), &data);
}

TEST(class_basic_ptr, release) {
    TestStruct data{10};
    basic_ptr<TestStruct> ptr(&data);
    auto* releasedPtr = ptr.release();
    ASSERT_EQ(releasedPtr, &data);
    ASSERT_EQ(ptr.get(), nullptr);
}

TEST(class_basic_ptr, operator_arrow) {
    TestStruct data{10};
    basic_ptr<TestStruct> ptr(&data);
    ASSERT_EQ(ptr->value, 10);
}

TEST(class_basic_ptr, operator_deref) {
    TestStruct rawStruct{10};
    basic_ptr<TestStruct> ptr(&rawStruct);
    ASSERT_EQ((*ptr).value, 10);
}

TEST(class_basic_ptr, bool_convert) {
    basic_ptr<TestStruct> ptr(new TestStruct{10});
    ASSERT_TRUE(ptr);

    basic_ptr<TestStruct> nullPtr(nullptr);
    ASSERT_FALSE(nullPtr);
}

TEST(class_basic_ptr, move_constructor) {
    basic_ptr<TestStruct> ptr(new TestStruct{10});
    basic_ptr<TestStruct> movedPtr(std::move(ptr));
    ASSERT_TRUE(movedPtr);
    ASSERT_FALSE(ptr);
}

TEST(class_basic_ptr, move_assignment) {
    basic_ptr<TestStruct> ptr(new TestStruct{10});
    basic_ptr<TestStruct> movedPtr = std::move(ptr);
    ASSERT_TRUE(movedPtr);
    ASSERT_FALSE(ptr);  // NOLINT(bugprone-use-after-move)
}
}

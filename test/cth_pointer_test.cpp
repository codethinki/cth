
#include <gtest/gtest.h>

#include "../cth/cth_pointer.hpp"

namespace cth {

struct TestStruct {
    int value;
};
TEST(move_ptr, comparison) {
    int a = 10;
    int b = 10;

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


TEST(move_ptr, constructor) {
    auto rawPtr = new TestStruct{10};
    move_ptr<TestStruct> ptr(rawPtr);
    ASSERT_EQ(ptr.get(), rawPtr);
}

TEST(move_ptr, destructor) {
    TestStruct data1{10};
    move_ptr<TestStruct> ptr(&data1);
    {
        [[maybe_unused]] move_ptr<TestStruct> ptr1(&data1);
    }
    // Destructor test relies on manual verification that no leaks are present
}

TEST(move_ptr, swap) {
    TestStruct data1{10};
    move_ptr<TestStruct> ptr(&data1);
    TestStruct data2{10};
    move_ptr<TestStruct> ptr1(&data1);
    move_ptr<TestStruct> ptr2(&data2);

    swap(ptr1, ptr2);

    ASSERT_EQ(ptr1.get(), &data2);
    ASSERT_EQ(ptr2.get(), &data1);
}

TEST(move_ptr, get) {
    TestStruct data{10};
    move_ptr<TestStruct> ptr(&data);
    ASSERT_EQ(ptr.get(), &data);
}

TEST(move_ptr, release) {
    TestStruct data{10};
    move_ptr<TestStruct> ptr(&data);
    auto* releasedPtr = ptr.release();
    ASSERT_EQ(releasedPtr, &data);
    ASSERT_EQ(ptr.get(), nullptr);
}

TEST(move_ptr, operator_arrow) {
    TestStruct data{10};
    move_ptr<TestStruct> ptr(&data);
    ASSERT_EQ(ptr->value, 10);
}

TEST(move_ptr, operator_deref) {
    TestStruct rawStruct{10};
    move_ptr<TestStruct> ptr(&rawStruct);
    ASSERT_EQ((*ptr).value, 10);
}

TEST(move_ptr, bool_convert) {
    move_ptr<TestStruct> ptr(new TestStruct{10});
    ASSERT_TRUE(ptr);

    move_ptr<TestStruct> nullPtr(nullptr);
    ASSERT_FALSE(nullPtr);
}

TEST(move_ptr, move_constructor) {
    move_ptr<TestStruct> ptr(new TestStruct{10});
    move_ptr<TestStruct> movedPtr(std::move(ptr));
    ASSERT_TRUE(movedPtr);
    ASSERT_FALSE(ptr);
}

TEST(move_ptr, move_assignment) {
    move_ptr<TestStruct> ptr(new TestStruct{10});
    move_ptr<TestStruct> movedPtr = std::move(ptr);
    ASSERT_TRUE(movedPtr);
    ASSERT_FALSE(ptr); // NOLINT(bugprone-use-after-move)
}
}

namespace cth::ptr {
TEST(protected_ptr, comparison) {
    struct asdf {
        int a = 10;
    };
    struct asdf2 : asdf {
        int b = 10;
    };
    asdf* a = new asdf{10};
    asdf2* b = new asdf2{10};

    const_ptr px = a < b ? a : b;
    const_ptr px2 = a < b ? a : b;
    const_ptr py = a > b ? a : b;


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

    const_ptr<TestStruct> nullPtr(nullptr);
    ASSERT_TRUE(nullPtr == nullptr);
    ASSERT_FALSE(nullPtr != nullptr);
    ASSERT_FALSE(nullPtr < nullptr);
    ASSERT_FALSE(nullPtr > nullptr);
    ASSERT_TRUE(nullPtr <= nullptr);
    ASSERT_TRUE(nullPtr >= nullptr);

    delete a;
    delete b;
}



TEST(protected_ptr, operator_arrow) {
    TestStruct data{10};
    const_ptr<TestStruct> ptr(&data);
    ASSERT_EQ(ptr->value, 10);
}

//TEST(protected_ptr, operator_deref) {
//    TestStruct rawStruct{10};
//    const_ptr<TestStruct> ptr(&rawStruct);
//    ASSERT_EQ((*ptr).value, 10);
//}

TEST(protected_ptr, bool_convert) {
    const_ptr<TestStruct> ptr = make_const<TestStruct>(10);
    ASSERT_TRUE(ptr);

    const_ptr<TestStruct> nullPtr(nullptr);
    ASSERT_FALSE(nullPtr);
}
}

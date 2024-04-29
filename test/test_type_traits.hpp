#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_type_traits.hpp"

//ik this test file is incomplete but idc

namespace cth {


#define TRAITS_TEST(trait, correct, wrong) \
    EXPECT_TRUE(type::trait##_v<correct>); \
    EXPECT_FALSE(type::trait##_v<wrong>)

#define DOUBLE_TRAITS_TEST(trait, correct1, correct2, wrong1, wrong2) \
    EXPECT_TRUE(type::trait##_v<correct1>); \
    EXPECT_TRUE(type::trait##_v<correct2>); \
    EXPECT_FALSE(type::trait##_v<wrong1>); \
    EXPECT_FALSE(type::trait##_v<wrong2>)

TEST(HeaderTypeTraits, TestChar) {
    TRAITS_TEST(is_nchar, char, wchar_t);


    TRAITS_TEST(is_wchar, wchar_t, char);

}
//TEST(HeaderTypeTraits, TestString) {
//    TRAITS_TEST(is_nstring, string, wstring);
//    TRAITS_TEST(is_wstring, wstring, string*);
//    TRAITS_TEST(is_string, string, string_view);
//}


} // namespace cth
#pragma once
#include <gtest/gtest.h>

#include "../cth/src/cth_type_traits.hpp"

//ik this test file is incomplete but idc

namespace cth {
using namespace std;

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
    DOUBLE_TRAITS_TEST(is_nchar_p, char*, char[10], wchar_t*, wchar_t[10]);

    TRAITS_TEST(is_wchar, wchar_t, char);
    DOUBLE_TRAITS_TEST(is_wchar_p, wchar_t*, wchar_t[10], wchar_t, const wchar_t);
}
TEST(HeaderTypeTraits, TestString) {
    TRAITS_TEST(is_nstring, string, wstring);
    TRAITS_TEST(is_wstring, wstring, string*);
    TRAITS_TEST(is_string, string, string_view);
}
TEST(HeaderTypeTraits, TestTextual) {
    TRAITS_TEST(is_ntextual, string, wstring);
    TRAITS_TEST(is_wtextual, wchar_t*, string);
    TRAITS_TEST(is_textual, string, char);
}

TEST(HeaderTypeTraits, TestLiteral) {
    DOUBLE_TRAITS_TEST(is_nliteral, string, char[10], int, double);
    DOUBLE_TRAITS_TEST(is_wliteral, wchar_t*, wchar_t, size_t, float);
}

TEST(HeaderTypeTraits, TestOstream) {
    DOUBLE_TRAITS_TEST(is_ostream, ostream, wostream, ifstream, istream);
}
}
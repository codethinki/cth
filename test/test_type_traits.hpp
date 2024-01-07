#pragma once
#include <gtest/gtest.h>

#include "../type_traits.hpp"

namespace cth {
#define TRAITS_TEST(trait, correct, wrong) \
    EXPECT_TRUE(type_traits::trait##_v<correct>); \
    EXPECT_FALSE(type_traits::trait##_v<wrong>)

#define DOUBLE_TRAITS_TEST(trait, correct1, correct2, wrong1, wrong2) \
    EXPECT_TRUE(type_traits::trait##_v<correct1>); \
    EXPECT_TRUE(type_traits::trait##_v<correct2>); \
    EXPECT_FALSE(type_traits::trait##_v<wrong1>); \
    EXPECT_FALSE(type_traits::trait##_v<wrong2>)

TEST(header_type_traits, char_tests) {
    TRAITS_TEST(is_nchar, char, wchar_t);
    DOUBLE_TRAITS_TEST(is_nchar_p, char*, char[10], wchar_t*, wchar_t[10]);

    TRAITS_TEST(is_wchar, wchar_t, char);
    DOUBLE_TRAITS_TEST(is_wchar_p, wchar_t*, wchar_t[10], wchar_t, const wchar_t);
}
TEST(header_type_traits, string_tests) {
    TRAITS_TEST(is_nstring, string, wstring);
    TRAITS_TEST(is_wstring, wstring, string*);
    TRAITS_TEST(is_string, string, string_view);
}
TEST(header_type_traits, textual_tests) {
    TRAITS_TEST(is_ntextual, string, wstring);
    TRAITS_TEST(is_wtextual, wchar_t*, string);
    TRAITS_TEST(is_textual, string, char);
}

TEST(header_type_traits, literal_tests) {
    DOUBLE_TRAITS_TEST(is_nliteral, string, char[10], int, double);
    DOUBLE_TRAITS_TEST(is_wliteral, wchar_t*, wchar_t, size_t, float);
}

TEST(header_type_traits, ostream_tests) {
    DOUBLE_TRAITS_TEST(is_ostream, ostream, wostream, ifstream, istream);
}
}
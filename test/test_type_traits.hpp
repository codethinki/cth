#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_type_traits.hpp"


//ik this test file is incomplete but idc

namespace cth::type {


#define TRAITS_TEST(trait, correct, wrong) \
    EXPECT_TRUE(type::trait##_v<correct>); \
    EXPECT_FALSE(type::trait##_v<wrong>)

#define DOUBLE_TRAITS_TEST(trait, correct1, correct2, wrong1, wrong2) \
    EXPECT_TRUE(type::trait##_v<correct1>); \
    EXPECT_TRUE(type::trait##_v<correct2>); \
    EXPECT_FALSE(type::trait##_v<wrong1>); \
    EXPECT_FALSE(type::trait##_v<wrong2>)

TEST(char_traits, value) {
    TRAITS_TEST(is_nchar, char, wchar_t);


    TRAITS_TEST(is_wchar, wchar_t, char);
}

TEST(trait_is_any_of, value) {
    EXPECT_TRUE((type::is_any_of_v<int, char, int, double>));
    EXPECT_TRUE((type::is_any_of_v<int, int, bool, std::string_view>));
    EXPECT_TRUE((type::is_any_of_v<int, bool, std::string_view, int>));
    EXPECT_FALSE((type::is_any_of_v<int, bool, std::string_view, float>));
}
TEST(trait_any_of, type) {
    int x = 10;
    using x_t = decltype(x);


    EXPECT_TRUE((std::is_same_v<x_t, type::any_of_t<x_t, bool, double, float, x_t>>));
    EXPECT_TRUE((std::is_same_v<x_t, type::any_of_t<x_t, x_t, bool, double, float>>));
    EXPECT_TRUE((std::is_same_v<x_t, type::any_of_t<x_t, bool, x_t, double, float>>));
    EXPECT_TRUE((std::is_same_v<void, type::fallback_any_of_t<void, x_t, bool, double, double, float>>));
}
TEST(trait_is_convertible_to_any_of, value) {

    EXPECT_TRUE((type::is_any_constructible_from_v<int, float, double>));
    EXPECT_TRUE((type::is_any_constructible_from_v<int, std::string_view, float, bool>));
    EXPECT_TRUE((type::is_any_constructible_from_v<int, std::string, std::string_view, float>));
    EXPECT_FALSE((type::is_any_constructible_from_v<int, std::wstring, std::string_view, std::ostream>));
}
TEST(trait_convert_any_of, type) {

    EXPECT_TRUE((std::is_same_v<bool, type::convert_to_any_t<bool, int, bool, double, float>>));
    EXPECT_TRUE((std::is_same_v<int, type::convert_to_any_t<int, bool, double, float, int>>));
    EXPECT_TRUE((std::is_same_v<double, type::convert_to_any_t<double, bool, float, double>>));
    EXPECT_TRUE((std::is_same_v<void, type::fallback_convert_to_any_t<void, double, std::string, std::string_view, std::wstring>>));
}
TEST(func_to_convertible, main) {
    
    double b = 10.0;
    [[maybe_unused]] constexpr bool y = std::is_constructible_v<int, double>;

    [[maybe_unused]] bool x = is_any_constructible_from_v<int, double>;

    auto z = to_constructible<std::string, std::string_view, int>(10);

    EXPECT_TRUE((std::is_same_v<decltype(z), int>));

    constexpr auto str = "hello world";
    constexpr char c = 'c';

    [[maybe_unused]] const auto charType = to_constructible<char, std::string_view, std::wstring_view>(c);
    [[maybe_unused]] const auto stringViewType = to_constructible<char, std::string_view, std::wstring_view>(str);
    
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(charType)>, char>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(stringViewType)>, std::string_view>));

    /*auto x = to_constructible<double>(10);*/
}
//TEST(HeaderTypeTraits, TestString) {
//    TRAITS_TEST(is_nstring, string, wstring);
//    TRAITS_TEST(is_wstring, wstring, string*);
//    TRAITS_TEST(is_string, string, string_view);
//}


} // namespace cth::type

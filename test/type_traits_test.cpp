#include "../cth/type_traits.hpp"

#include <array>
#include <print>
#include <span>
#include <vector>

#include <gtest/gtest.h>

namespace cth::type {



TEST(apply_trait, main) {
    using T = int****;


    EXPECT_TRUE((std::same_as<apply_trait_t<T, std::remove_pointer_t>, int***>));
    EXPECT_TRUE((std::same_as<apply_trait_t<T, std::remove_pointer_t, 4>, int>));
}

TEST(trait_count, main) {
    using T = std::vector<std::vector<std::vector<int>>>;

    constexpr size_t dimensions = trait_count<T, std::ranges::range_value_t>();
    EXPECT_EQ(dimensions, 3);


    constexpr size_t dimensions2 = trait_count<T, std::ranges::range_value_t, 2>();
    EXPECT_EQ(dimensions2, 2);

    constexpr size_t dimensions3 = trait_count<int, std::ranges::range_value_t>();
    EXPECT_EQ(dimensions3, 0);

    constexpr size_t dimensions4 = trait_count<T, std::ranges::range_value_t, 0>();
    EXPECT_EQ(dimensions4, 0);
}

template<class T>
concept pointer_type = std::is_pointer_v<T>;

TEST(cpt_count, main) {
    using T = int****;

    constexpr size_t ptrDimensions = cpt_count<T, CPT(pointer_type), std::remove_pointer_t>();
    EXPECT_EQ(ptrDimensions, 4);

    constexpr size_t ptrDimensions2 = cpt_count<T, CPT(pointer_type), std::remove_pointer_t, 3>();
    EXPECT_EQ(ptrDimensions2, 3);

    constexpr size_t ptrDimensions3 = cpt_count<T, CPT(pointer_type), std::ranges::range_value_t, 0>();
    EXPECT_EQ(ptrDimensions3, 0);
}



using md_range_t = std::vector<std::array<std::span<std::vector<std::vector<uint32_t>>>, 10>>;

TEST(md_range, type_trait) {
    static_assert(type::md_range<md_range_t, 5>, "failed to evaluate is_md_range_v");
    static_assert(std::same_as<md_range_value_t<md_range_t, 5>, uint32_t>, "failed to evaluate md_range_val_t");
}



TEST(is_any_of, value) {
    EXPECT_TRUE((type::is_any_of<int, char, int, double>));
    EXPECT_TRUE((type::is_any_of<int, int, bool, std::string_view>));
    EXPECT_TRUE((type::is_any_of<int, bool, std::string_view, int>));
    EXPECT_FALSE((type::is_any_of<int, bool, std::string_view, float>));
}
TEST(any_of, type) {
    int x = 10;
    using x_t = decltype(x);


    EXPECT_TRUE((std::is_same_v<x_t, type::any_of_t<x_t, bool, double, float, x_t>>));
    EXPECT_TRUE((std::is_same_v<x_t, type::any_of_t<x_t, x_t, bool, double, float>>));
    EXPECT_TRUE((std::is_same_v<x_t, type::any_of_t<x_t, bool, x_t, double, float>>));
    EXPECT_TRUE((std::is_same_v<void, type::fallback_any_of_t<void, x_t, bool, double, double, float>>));
}
TEST(is_any_constructible_from, value) {

    EXPECT_TRUE((type::any_constructible_from<int, float, double>));
    EXPECT_TRUE((type::any_constructible_from<int, std::string_view, float, bool>));
    EXPECT_TRUE((type::any_constructible_from<int, std::string, std::string_view, float>));
    EXPECT_FALSE((type::any_constructible_from<int, std::wstring, std::string_view, std::ostream>));
}
TEST(convert_to_any, type) {

    EXPECT_TRUE((std::is_same_v<bool, type::convert_to_any_t<bool, int, bool, double, float>>));
    EXPECT_TRUE((std::is_same_v<int, type::convert_to_any_t<int, bool, double, float, int>>));
    EXPECT_TRUE((std::is_same_v<double, type::convert_to_any_t<double, bool, float, double>>));
    EXPECT_TRUE((std::is_same_v<void, type::fallback_convert_to_any_t<void, double, std::string, std::string_view, std::wstring>>));
}
TEST(to_convertible, main) {

    double b = 10.0;
    [[maybe_unused]] constexpr bool y = std::is_constructible_v<int, double>;

    [[maybe_unused]] bool x = any_constructible_from<int, double>;

    auto z = to_constructible<std::string, std::string_view, int>(10);

    EXPECT_TRUE((std::is_same_v<decltype(z), int>));

    constexpr auto str = "hello world";
    constexpr char c = 'c';

    [[maybe_unused]] auto const charType = to_constructible<char, std::string_view, std::wstring_view>(c);
    [[maybe_unused]] auto const stringViewType = to_constructible<char, std::string_view, std::wstring_view>(str);

    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(charType)>, char>));
    EXPECT_TRUE((std::is_same_v<std::decay_t<decltype(stringViewType)>, std::string_view>));

    /*auto x = to_constructible<double>(10);*/
}

using md_range_t = std::vector<std::array<std::span<std::vector<std::vector<uint32_t>>>, 10>>;
constexpr size_t D = 5;

TEST(md_range, concepts) {
    static_assert(md_range<md_range_t, D>);
    static_assert(md_range_over<md_range_t, uint32_t, D>);
    static_assert(md_range_over<md_range_t, uint32_t>);
    static_assert(md_range_over<md_range_t, std::vector<uint32_t>, D - 1>);
    static_assert(md_range_over_cpt<md_range_t, CPT(std::integral), D>);
}

} // namespace cth::value_type

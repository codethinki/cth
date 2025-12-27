#include "test.hpp"

#include "cth/meta/ranges.hpp"

#include "cth/meta/concepts.hpp"


using Base = int;
using Prev = std::array<Base, 10>;
using T = std::array<std::vector<Prev>, 20>;
constexpr size_t rank = 3;


namespace cth::type {

TYPE_TEST(dimensions, exact) {
    constexpr auto result = dimensions<T, rank>();

    ASSERT_EQ(result, rank);
}

TYPE_TEST(dimensions, no_limit) {
    constexpr auto result = dimensions<T>();

    ASSERT_EQ(result, rank);
}

TYPE_TEST(dimensions, limited) {
    constexpr auto max = rank - 1;

    constexpr auto result = dimensions<T, max>();

    ASSERT_EQ(result, max);
}
}


namespace cth::type {
TYPE_TEST(md_range, true) {
    ASSERT_TRUE((md_range<T, rank>));

    ASSERT_TRUE((md_range<T, rank - 1>));
}
TYPE_TEST(md_range, false) {
    ASSERT_FALSE((md_range<T, rank + 1>));
}
}


namespace cth::type {
TYPE_TEST(md_range_value_t, exact) {
    ASSERT_TRUE((std::same_as<md_range_value_t<T, rank>, Base>));
}
TYPE_TEST(md_range_value_t, smaller) {
    ASSERT_TRUE((std::same_as<md_range_value_t<T, rank - 1>, Prev>));
}
TYPE_TEST(md_range_value_t, larger) {
    ASSERT_TRUE((std::same_as<md_range_value_t<T>, Base>));
}
}


namespace cth::type {
TYPE_TEST(md_range_over, exact) {
    ASSERT_TRUE((md_range_over<T, Base, rank>));
}
TYPE_TEST(md_range_over, smaller) {
    ASSERT_TRUE((md_range_over<T, Prev, rank - 1>));
}
TYPE_TEST(md_range_over, larger) {
    ASSERT_TRUE((md_range_over<T, Base>));
}
}
namespace cth::type {
TYPE_TEST(md_range_over_cpt, exact) {
    ASSERT_TRUE((md_range_over_cpt<T, CPT(std::same_as<Base>), rank>));
}
TYPE_TEST(md_range_over_cpt, smaller) {
    ASSERT_TRUE((md_range_over_cpt<T, CPT(std::same_as<Prev>), rank - 1>));
}
TYPE_TEST(md_range_over_cpt, larger) {
    ASSERT_TRUE((md_range_over_cpt<T, CPT(std::same_as<Base>)>));
}
}

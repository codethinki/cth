// ReSharper disable CppClangTidyCppcoreguidelinesAvoidMagicNumbers
#include "../cth/concepts.hpp"

#include <array>
#include <span>
#include <vector>

#include <gtest/gtest.h>


//this header is missing tests but idc because it's just concepts

namespace cth::type {

using md_range_t = std::vector<std::array<std::span<std::vector<std::vector<uint32_t>>>, 10>>;

TEST(md_range, concepts) {
    static_assert(md_range<md_range_t, 5>);
    static_assert(md_range_over<md_range_t, 5, uint32_t>);
    static_assert(md_range_over<md_range_t, 4, std::vector<uint32_t>>);
    static_assert(md_range_over_cpt<md_range_t, 5, cpt(std::integral)>);
}

}

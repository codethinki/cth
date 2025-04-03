// ReSharper disable CppClangTidyReadabilityContainerDataPointer
// ReSharper disable CppClangTidyCppcoreguidelinesAvoidMagicNumbers
#include "cth/test.hpp"

#include "cth/algorithm.hpp"
#include "cth/string.hpp"
#define VIEWS_TEST(suite, test_name) CTH_EX_TEST(_views, suite, test_name)

namespace cth::views {

VIEWS_TEST(to_ptr_range, pipe) {
    std::vector values{0, 1, 2};
    std::vector const expected{&values[0], &values[1], &values[2]};

    auto const pipePtrs = values | to_ptr_range;
    auto const callPtrs = to_ptr_range(values);

    for(auto const [actual, expected] : std::views::zip(callPtrs, expected))
        ASSERT_EQ(actual, expected);
    for(auto const [actual, left] : std::views::zip(pipePtrs, expected))
        ASSERT_EQ(actual, left);
}

VIEWS_TEST(to_ptr_range, const_type) {
    using base_t = int const;
    using expected_t = base_t*;
    using range_t = std::span<base_t>; // NOLINT(portability-std-allocator-const)

    using call_actual_t = type::md_range_value_t<decltype(to_ptr_range(std::declval<range_t>()))>;
    using pipe_actual_t = type::md_range_value_t<decltype(std::declval<range_t>() | to_ptr_range)>;

    // ReSharper disable once CppIdenticalOperandsInBinaryExpression
    static_assert(std::same_as<expected_t, call_actual_t> && std::same_as<expected_t, pipe_actual_t>,
        "to_ptr_range type mismatch, expected int const");

}

VIEWS_TEST(split_into_fn, even) {}

namespace dev {

    static void test_split_info_fn(auto&& values, int split_into, int expected_chunks) {
        auto const expectedChunks = values | std::views::chunk(expected_chunks);
        auto const actualChunks = values | cth::views::split_into(split_into);
        EXPECT_RANGE_EQ(actualChunks, expectedChunks);
    }
}

VIEWS_TEST(split_into_fn, split_size) {
    std::vector oddSizeValues{0, 1, 2, 3, 4};
    std::vector evenSizeValues{0, 1, 2, 3, 4, 5};

    dev::test_split_info_fn(evenSizeValues, 2, 3);
    dev::test_split_info_fn(oddSizeValues, 2, 3);
}

}

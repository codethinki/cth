// ReSharper disable CppClangTidyReadabilityContainerDataPointer
// ReSharper disable CppClangTidyCppcoreguidelinesAvoidMagicNumbers
#include "cth/test.hpp"

import cth.alg.views;
import cth.typ.ranges;
import cth.string;

#define VIEWS_TEST(suite, test_name) CTH_EX_TEST(_views, suite, test_name)

namespace cth::views {

VIEWS_TEST(to_ptr_range, pipe) {
    std::vector values{0, 1, 2};
    std::vector const expected{&values[0], &values[1], &values[2]};

    auto const pipePtrs = values | to_ptr_range;
    auto const callPtrs = to_ptr_range(values);

    for(auto const [a, e] : std::views::zip(callPtrs, expected))
        ASSERT_EQ(a, e);
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

namespace {
    void test_split_info_fn(auto&& values, int split_into, int expected_chunks) {
        auto const expectedChunks = values | std::views::chunk(expected_chunks);
        auto const actualChunks = values | cth::views::split_into(split_into);
        EXPECT_RANGE_EQ(actualChunks, expectedChunks);
    }
}

VIEWS_TEST(split_into_fn, split_size) {
    std::vector evenSizeValues{0, 1, 2, 3, 4, 5};
    std::vector const oddSizeValues{0, 1, 2, 3, 4};


    test_split_info_fn(evenSizeValues, 2, 3);
    test_split_info_fn(oddSizeValues, 2, 3);
}

namespace {
    template<std::ranges::range T, class U>
    void test_drop_stride_fn(T&& values, U&& expected, int offset, int stride) {
        EXPECT_RANGE_EQ(
            (std::forward<T>(values) | views::drop_stride(offset, stride)),
            std::forward<U>(expected)
        );
    }
    
}

VIEWS_TEST(drop_stride_fn, asdf1) {


    test_drop_stride_fn(std::vector{0, 1, 2, 3}, std::vector{1, 3}, 1, 2);
    test_drop_stride_fn(std::vector{0, 1, 2, 3}, std::vector{0, 2}, 0, 2);
}

}

namespace cth::views {
struct A {
    A() = default;
    A(int asdf) : asdf{asdf} {}

    int foo(int a) const { return asdf + a; }
    double f(int, double d, float) { return asdf + d; }
    static size_t x(A const& a) { return a.asdf; }

    int asdf;
};

std::vector<A> values{{0}, {1}, {2}, {3}, {4}};


VIEWS_TEST(transform_call_fn, member_fn_singe) {
    std::vector<int> expected{1, 2, 3, 4, 5};

    std::vector actual{std::from_range, values | views::transform_call(&A::foo, 1)};
}
}

#include "../cth/cth_algorithm.hpp"
#include "../cth/io/cth_log.hpp"

#include <gtest/gtest.h>



namespace cth::algorithm {
using std::vector;

TEST(uniqueCombine, success) {
    //success
    const vector<vector<uint32_t>> input = {{5, 5, 5}, {5, 5, 4}, {5, 5, 3}};
    const auto solution = vector<uint32_t>{5, 4, 3};
    EXPECT_EQ(uniqueCombine(input), solution);
}
TEST(uniqueCombine, fail) {
    //fail
    const vector<vector<uint32_t>> input2 = {{5, 5, 5}, {5, 5, 5, 5, 5}};
    const auto solution2 = vector<uint32_t>{};
    EXPECT_EQ(uniqueCombine(input2), solution2);
}

TEST(assign, main) {
    //success

    auto test = [](const vector<vector<size_t>>& a_b_options, const vector<size_t>& b_max, const vector<size_t>& expected) {
        const vector<size_t> result = assign(a_b_options, b_max);
        const bool equal = result == expected;
        CTH_STABLE_ERR(!equal, "assign test failed") {
            details->add("expected: {}", cth::str::to_string(expected));
            details->add("actual: {}", cth::str::to_string(result));
        }
        EXPECT_TRUE(equal);
    };

    test({{0, 1}, {0}, {0, 1, 2}}, {1, 2, 0}, {1, 0, 1});
    test({{0, 1}, {0}, {0, 1, 2}, {0}}, {2, 1, 1}, {1, 0, 2, 0});
    test({{0, 1}, {0, 1}}, {3, 2, 1, 3}, {0, 0});
    test({{0, 4}, {0, 4}}, {0, 3, 3, 3, 2}, {4, 4});
}
TEST(assign, fail) {
    //fail
    const std::array<vector<size_t>, 3> input = {{{1, 2, 0}, {2}, {0, 1}}};
    const std::array<size_t, 3> max = {1, 2, 0};
    const auto solution = assign(input, max);
    EXPECT_TRUE(solution.empty());
}

} // namespace cth

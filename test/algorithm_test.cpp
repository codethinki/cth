#include "../cth/algorithm.hpp"
#include "../cth/io/log.hpp"

#include <gtest/gtest.h>



namespace cth::algorithm {
using std::vector;

TEST(uniqueCombine, success) {
    //success
    vector<vector<uint32_t>> const input = {{5, 5, 5}, {5, 5, 4}, {5, 5, 3}};
    auto const solution = vector<uint32_t>{5, 4, 3};
    EXPECT_EQ(uniqueCombine(input), solution);
}
TEST(uniqueCombine, fail) {
    //fail
    vector<vector<uint32_t>> const input2 = {{5, 5, 5}, {5, 5, 5, 5, 5}};
    auto const solution2 = vector<uint32_t>{};
    EXPECT_EQ(uniqueCombine(input2), solution2);
}

TEST(assign, main) {
    //success

    auto test = [](vector<vector<size_t>> const& a_b_options, vector<size_t> const& b_max, vector<size_t> const& expected) {
        vector<size_t> const result = assign(a_b_options, b_max);
        bool const equal = result == expected;
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
    test({{0}}, {16, 2, 8, 1, 1, 1}, {0});
}
TEST(assign, fail) {
    //fail
    std::array<vector<size_t>, 3> const input = {{{1, 2, 0}, {2}, {0, 1}}};
    std::array<size_t, 3> const max = {1, 2, 0};
    auto const solution = assign(input, max);
    EXPECT_TRUE(solution.empty());
}

} // namespace cth

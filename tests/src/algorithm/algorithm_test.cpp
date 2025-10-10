#include "cth/test.hpp"

#include "cth/algorithm.hpp"
#include "cth/string.hpp"
#include "cth/algorithm/combine.hpp"

#include <vector>


#define ALG_TEST(suite, test_name) CTH_EX_TEST(_algorithm, suite, test_name)



namespace cth::alg {
using std::vector;

ALG_TEST(unique_combine, success) {
    //success
    vector<vector<uint32_t>> const input = {{5, 5, 5}, {5, 5, 4}, {5, 5, 3}};
    auto const solution = vector<uint32_t>{5, 4, 3};
    EXPECT_EQ(unique_combine(input), solution);
}
ALG_TEST(unique_combine, fail) {
    //fail
    vector<vector<uint32_t>> const input2 = {{5, 5, 5}, {5, 5, 5, 5, 5}};
    auto const solution2 = vector<uint32_t>{};
    EXPECT_EQ(unique_combine(input2), solution2);
}

ALG_TEST(assign, main) {
    auto test = [](vector<vector<size_t>> const& a_b_options, vector<size_t> const& b_max, vector<size_t> const& expected) {
        vector<size_t> const result = cth::alg::assign(a_b_options, b_max);
        ASSERT_EQ(result, expected);
    };

    test({{0, 1}, {0}, {0, 1, 2}}, {1, 2, 0}, {1, 0, 1});
    test({{0, 1}, {0}, {0, 1, 2}, {0}}, {2, 1, 1}, {1, 0, 2, 0});
    test({{0, 1}, {0, 1}}, {3, 2, 1, 3}, {0, 0});
    test({{0, 4}, {0, 4}}, {0, 3, 3, 3, 2}, {4, 4});
    test({{0}}, {16, 2, 8, 1, 1, 1}, {0});
}
ALG_TEST(assign, fail) {
    //fail
    std::array<vector<size_t>, 3> const input = {{{1, 2, 0}, {2}, {0, 1}}};
    std::array<size_t, 3> const max = {1, 2, 0};
    auto const solution = assign(input, max);
    EXPECT_TRUE(solution.empty());
}

}

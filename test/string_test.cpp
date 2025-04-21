#include "../cth/string.hpp"

#include "cth/test.hpp"

#define STR_TEST(suite, test_name) CTH_EX_TEST(_str, suite, test_name)

namespace cth::str {
using std::nullopt;

STR_TEST(to_string, range) {
    std::vector<int> const vec = {1, 2, 3, 4, 5};
    std::string const str = cth::str::to_string(vec);
    EXPECT_EQ(str, "[1, 2, 3, 4, 5]");
}

STR_TEST(to_string, 2drange) {
    std::vector<std::vector<int>> const vec = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    std::string const str = cth::str::to_string(vec);
    EXPECT_EQ(str, "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");
}

STR_TEST(to_string, view) {
    std::vector<int> const vec = {1, 2, 3, 4, 5};

    std::string const str = cth::str::to_string(vec);
    EXPECT_EQ(str, "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");
}


STR_TEST(to_num, main) {
    EXPECT_EQ(cth::str::to_num<uint32_t>("1234"), 1234u);
    EXPECT_EQ(cth::str::to_num<float>("1234"), 1234.f);
    EXPECT_EQ(cth::str::to_num<float>("1234.5"), 1234.5f);
    EXPECT_EQ(cth::str::to_num<double>("8402.81237"), 8402.81237);

    EXPECT_EQ(cth::str::to_num<uint32_t>("1hH23a4"), nullopt);
    EXPECT_EQ(cth::str::to_num<float>("1as234"), nullopt);
    EXPECT_EQ(cth::str::to_num<float>("1234..5"), nullopt);
    EXPECT_EQ(cth::str::to_num<double>("8402.81A237"), nullopt);
}

STR_TEST(split, main) {


    auto res = str::split("asdf  asdf asdf dht", " ");

    cxpr std::array<std::string_view, 4> validRes = {{"asdf", "asdf", "asdf", "dht"}};

    for(size_t i = 0; i < res.size(); ++i)
        EXPECT_EQ(res[i], validRes[i]);

    [[maybe_unused]] auto res2 = str::split(L"asdf  asdf asdf dht", L' ');

    for(size_t i = 0; i < res.size(); ++i)
        EXPECT_EQ(res[i], validRes[i]);


}

} // namespace cth

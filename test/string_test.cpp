#include "../cth/string.hpp"

#include <gtest/gtest.h>


namespace cth::str {
using std::nullopt;

TEST(to_string, range) {
    std::vector<int> const vec = {1, 2, 3, 4, 5};
    std::string const str = cth::str::to_string(vec);
    EXPECT_EQ(str, "[1, 2, 3, 4, 5]");
}

TEST(to_string, 2drange) {
    std::vector<std::vector<int>> const vec = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    std::string const str = cth::str::to_string(vec);
    EXPECT_EQ(str, "[[1, 2, 3], [4, 5, 6], [7, 8, 9]]");
}

TEST(convert, wide_narrow) {
    EXPECT_EQ(cth::str::conv::toN(L"asdf"), "asdf");
    EXPECT_EQ(cth::str::conv::toW("asdf"), L"asdf");
}

TEST(to_num, main) {
    EXPECT_EQ(cth::str::to_num<uint32_t>("1234"), 1234u);
    EXPECT_EQ(cth::str::to_num<float>("1234"), 1234.f);
    EXPECT_EQ(cth::str::to_num<float>("1234.5"), 1234.5f);
    EXPECT_EQ(cth::str::to_num<double>("8402.81237"), 8402.81237);

    EXPECT_EQ(cth::str::to_num<uint32_t>("1hH23a4"), nullopt);
    EXPECT_EQ(cth::str::to_num<float>("1as234"), nullopt);
    EXPECT_EQ(cth::str::to_num<float>("1234..5"), nullopt);
    EXPECT_EQ(cth::str::to_num<double>("8402.81A237"), nullopt);

    EXPECT_EQ(cth::str::to_num<uint32_t>(L"1234"), 1234u);
    EXPECT_EQ(cth::str::to_num<float>(L"1234"), 1234.f);
    EXPECT_EQ(cth::str::to_num<float>(L"1234.5"), 1234.5f);
    EXPECT_EQ(cth::str::to_num<double>(L"8402.81237"), 8402.81237);

    EXPECT_EQ(cth::str::to_num<uint32_t>(L"1hH23a4"), nullopt);
    EXPECT_EQ(cth::str::to_num<float>(L"1as234"), nullopt);
    EXPECT_EQ(cth::str::to_num<float>(L"1234..5"), nullopt);
    EXPECT_EQ(cth::str::to_num<double>(L"8402.81ABN237"), nullopt);
}

TEST(split, main) {


    auto res = str::split("asdf  asdf asdf dht", " ");

    constexpr std::array<std::string_view, 4> validRes = {{"asdf", "asdf", "asdf", "dht"}};

    for(size_t i = 0; i < res.size(); ++i)
        EXPECT_EQ(res[i], validRes[i]);

    [[maybe_unused]] auto res2 = str::split(L"asdf  asdf asdf dht", L' ');

    for(size_t i = 0; i < res.size(); ++i)
        EXPECT_EQ(res[i], validRes[i]);


}

} // namespace cth

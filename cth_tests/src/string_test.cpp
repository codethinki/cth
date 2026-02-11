#include "cth/string.hpp"

#include "cth/test.hpp"

#include <filesystem>

#define STR_TEST(suite, test_name) CTH_EX_TEST(_str, suite, test_name)

namespace cth::str {
using std::nullopt;

STR_TEST(to_string, range_decline_nonstatic_dim) {
    std::filesystem::path x{"asdf.img"};
    auto const& a = x;

    EXPECT_FALSE(cth::rng::static_dim_rng<std::filesystem::path>);
    EXPECT_FALSE(cth::rng::static_dim_rng<std::filesystem::path const&>);
    EXPECT_FALSE(cth::rng::static_dim_rng<std::filesystem::path&>);
}

STR_TEST(to_string, value_only_rng) {
    static cxpr size_t COUNT = 10;

    std::map<size_t, size_t> map{};
    for(size_t i = 0; i < COUNT; i++)
        map[i] = i;

    auto view = map | std::views::keys | std::views::filter([](auto const id) { return id % 2 == 0; });

    auto const result = std::format("{}", view);
    EXPECT_EQ("cth::str::printable_rng{[0, 2, 4, 6, 8]}", result);
}

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

    std::string const str = cth::str::to_string(std::views::all(vec));
    EXPECT_EQ(str, "[1, 2, 3, 4, 5]");
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

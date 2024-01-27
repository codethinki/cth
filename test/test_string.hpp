#pragma once
#include <gtest/gtest.h>
#include "../cth/cth_string.hpp"

namespace cth {

TEST(headerString, namespaceConv) {
    EXPECT_EQ(cth::str::conv::toN(L"asdf"), "asdf");
    EXPECT_EQ(cth::str::conv::toW("asdf"), L"asdf");
}

TEST(headerString, funcToNum) {
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

TEST(headerString, funcSplit) {


    auto res = str::split("asdf  asdf asdf dht", " ");

    const array<string_view, 4> validRes = {{"asdf", "asdf", "asdf", "dht"}};

    for(size_t i = 0; i < res.size(); ++i)
        EXPECT_EQ(res[i], validRes[i]);

    auto res2 = str::split(L"asdf  asdf asdf dht", L' ');

    const array<wstring_view, 4> validRes2 = {{L"asdf", L"asdf", L"asdf", L"dht"}};

    for(size_t i = 0; i < res.size(); ++i)
        EXPECT_EQ(res[i], validRes[i]);


}

} // namespace cth

#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_windows.hpp"


namespace cth {


TEST(headerWindows, funcIsElevatedProc) {
    EXPECT_FALSE(cth::win::proc::elevated());

    win::cmd::hidden("asdfasdf");
    win::cmd::hidden("asdfaasdf {}", "asdf");

    win::cmd::hidden_dir(std::filesystem::current_path().string(), "asdfasdf");
    win::cmd::hidden_dir(std::filesystem::current_path().string(), "asdfasdf {}", "hello");
}
TEST(headerWindows, funcLoadTxt) {
   
    try {
        auto text = win::file::loadTxt("hello.txt");
        if(!text.empty())cout << text[0];
        auto wtext = win::file::loadTxt(L"hello.txt");
    }
    catch(...) {}
}
}

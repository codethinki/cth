#include "../../cth/io.hpp"
#include <gtest/gtest.h>


namespace cth {

 //namespace cth
TEST(loadTxt, main) {
    try {
        [[maybe_unused]] auto const text = cth::io::readText("hello.txt");
        [[maybe_unused]] auto text2 = cth::io::readText(L"hello.txt");
    }
    catch(...) {
        std::terminate();
    }
}
}


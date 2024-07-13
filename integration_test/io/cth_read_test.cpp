#include "../../cth/cth_io.hpp"
#include <gtest/gtest.h>


namespace cth {

 //namespace cth
TEST(loadTxt, main) {
    try {
        [[maybe_unused]] const auto text = cth::io::loadTxt("hello.txt");
        [[maybe_unused]] auto text2 = cth::io::loadTxt(L"hello.txt");
    }
    catch(...) {
        std::terminate();
    }
}
}


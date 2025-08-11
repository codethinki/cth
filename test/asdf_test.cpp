#include <filesystem>
#include <print>

#include <gtest/gtest.h>

#include "cth/string.hpp"

namespace cth {
TEST(asdf, asf) {
    std::filesystem::path path{"asdf.img"};
    std::print("{}", path);
}
}

#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_concepts.hpp"

//this header is missing tests but idc because it's just concepts

namespace cth::concepts {

TEST(concepts, main) {
    using namespace cth::type;

    EXPECT_FALSE(type::arithmetic<char*>);
}

}

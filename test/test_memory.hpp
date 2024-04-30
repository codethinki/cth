#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_memory.hpp"

namespace cth {
    class A{};
class B : public A{};

    TEST(memory, basicptr) {
        int x = 10;
        std::unique_ptr<>

        memory::basic_ptr px = &x;
        EXPECT_EQ(*px, 10);
        EXPECT_EQ(px.get(), &x);
        EXPECT_TRUE(px == &x);
        EXPECT_TRUE(&x == px);

    }
}

#pragma once
#include <gtest/gtest.h>

#include "../cth/cth_windows.hpp"


namespace cth {
TEST(headerWindows, funcIsElevatedProc) { EXPECT_FALSE(cth::win::proc::is_elevated()); }
    //TODO implement tests

}

#pragma once
#include <gtest/gtest.h>

#include "../cth/src/cth_windows.hpp"


namespace cth {
TEST(headerWindows, funcIsElevatedProc) { EXPECT_FALSE(cth::win::proc::is_elevated()); }


}

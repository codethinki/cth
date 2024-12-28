#pragma once
#include "cth/test.hpp"

#define WIN_EX_TEST(spacer, suite, test_name) S_TEST(_win##spacer, suite, test_name)
#define WIN_TEST(suite, test_name) WIN_EX_TEST(, suite, test_name)

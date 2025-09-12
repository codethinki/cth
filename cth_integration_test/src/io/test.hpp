#pragma once
#include "cth/test.hpp"

#define IO_EX_TEST(spacer, suite, test_name) CTH_EX_TEST(_io##spacer, suite, test_name)
#define IO_TEST(suite, test_name) IO_EX_TEST(, suite, test_name)

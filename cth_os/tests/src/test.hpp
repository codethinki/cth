#pragma once
#include "cth/test.hpp"

#define OS_EX_TEST(spacer, suite, test_name) CTH_EX_TEST(_os##spacer, suite, test_name)
#define OS_TEST(suite, test_name) OS_EX_TEST(, suite, test_name)

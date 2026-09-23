#pragma once
#include "cth/test.hpp"

#define NET_EX_TEST(spacer, suite, test_name) CTH_EX_TEST(_net##spacer, suite, test_name)
#define NET_TEST(suite, test_name) NET_EX_TEST(, suite, test_name)

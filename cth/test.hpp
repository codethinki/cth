#pragma once
#include <gtest/gtest.h>

#define S_TEST(spacer, suite, test_name) TEST(spacer##_##suite, test_name)

#define CTH_EX_TEST(spacer, suite, test_name) S_TEST(cth##spacer, suite, test_name)
#define CTH_TEST(suite, test_name) CTH_EX_TEST(, suite, test_name)
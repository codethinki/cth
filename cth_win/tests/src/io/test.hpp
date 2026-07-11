#pragma once
#include "../test.hpp"

#define WIN_IO_EX_TEST(ex, suite, test_name) WIN_EX_TEST(_io##ex, suite, test_name)
#define WIN_IO_TEST(suite, test_name) WIN_IO_EX_TEST(, suite, test_name)

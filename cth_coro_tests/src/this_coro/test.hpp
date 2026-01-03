#pragma once

#include "../test.hpp"

#define THIS_CORO_EX_TEST(ex, suite, name) CORO_EX_TEST(_this_coro##ex, suite, name)

#define THIS_CORO_TEST(suite, name) THIS_CORO_EX_TEST(, suite, name)

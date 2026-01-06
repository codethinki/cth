#pragma once

#include <cth/test.hpp>


#include <cth/meta/debug.hpp>


#define CORO_TEST(suite, name) CTH_EX_TEST(_coro, suite, name)
#define CORO_EX_TEST(ex, suite, name) CTH_EX_TEST(_coro##ex, suite, name)

#define CORO_ITEST(suite, name) CORO_EX_TEST(_i, suite, name)

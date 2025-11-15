#pragma once

#include <cth/test.hpp>

#define CORO_TEST(suite, name) CTH_EX_TEST(_coro, suite, name)
#define CORO_EX_TEST(ex, suite, name) CTH_EX_TEST(_coro##ex, suite, name)

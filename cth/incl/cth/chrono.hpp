#pragma once
#include <chrono>

namespace cth::chrono {
using clock_t = std::chrono::steady_clock;
using time_point_t = clock_t::time_point;

auto const session_start = clock_t::now();

}

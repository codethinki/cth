module;
#include <chrono>

export module cth.chrono;


export namespace cth::co {
using clock_t = std::chrono::steady_clock;
using time_point_t = clock_t::time_point;
}


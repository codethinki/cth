#pragma once
#include <type_traits>

namespace cth::concepts {
using namespace std;

template<typename T>
concept arithmetic_t = is_arithmetic_v<T>;

template<typename T>
concept floating_point_t = is_floating_point_v<T>;

template<typename T>
concept integral_t = is_integral_v<T>;

template<typename T>
concept unsigned_t = is_unsigned_v<T>;
}

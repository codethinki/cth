#pragma once
#include "cth_type_traits.hpp"

namespace cth::type {

template<typename T>
concept arithmetic_t = std::is_arithmetic_v<T>;

template<typename T>
concept floating_point_t = std::is_floating_point_v<T>;

template<typename T>
concept integral_t = std::is_integral_v<T>;

template<typename T>
concept unsigned_integral_t = std::is_unsigned_v<T>;

template<typename T>
concept char_t = std::_Is_any_of_v<std::remove_cv_t<T>, char, wchar_t>;


template<typename T>
concept string_view_convertable_t = is_string_view_convertable_v<T>;

template<typename T>
concept literal_t = is_string_view_convertable_v<T> || is_char_v<T>;

} // namespace cth::type



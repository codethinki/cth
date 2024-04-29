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

//template<typename T>
//concept string_t = is_string_v<T>;


//template<typename T>
//concept ntextual_t = is_ntextual_v<T>;
//template<typename T>
//concept wtextual_t = is_wtextual_v<T>;
//template<typename T>
//concept textual_t = is_textual_v<T>;
//
//
//template<typename T>
//concept nliteral_t = is_nliteral_v<T>;
//template<typename T>
//concept wliteral_t = is_wliteral_v<T>;
//template<typename T>
//concept literal_t = is_literal_v<T>;

template<typename T>
concept string_view_t = is_string_view_convertable_v<T>;

template<typename T>
concept literal_t = is_string_view_convertable_v<T> || is_char_v<T>;

//template<typename T>
//concept ostream_t = type::is_ostream_v<T>;

} // namespace cth::type

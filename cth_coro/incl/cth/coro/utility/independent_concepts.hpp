#pragma once
#include <concepts>
#include <type_traits>

namespace cth::co {
template<class P>
concept payload = std::copy_constructible<std::remove_cvref_t<P>>;

template<class T>
concept has_payload_type = requires { typename std::remove_cvref_t<T>::payload_type; };
}

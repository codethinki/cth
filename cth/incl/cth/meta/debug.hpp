#pragma once
#include <utility>


namespace cth::type {

template<class T>
using print_t = std::type_identity_t<T>;
}

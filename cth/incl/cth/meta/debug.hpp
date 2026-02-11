#pragma once
#include <utility>


namespace cth::mta {

template<class T>
using print_t = std::type_identity_t<T>;

namespace dev {
    template<class T>
    consteval auto collapse() {
        return std::type_identity<T>{};
    }
}

template<class T>
using collapse_t = decltype(dev::collapse<T>())::type;


}

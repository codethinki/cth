module;
#include <utility>

export module cth.meta.debug;


export namespace cth::mta {

template<class T>
using print_t = std::type_identity_t<T>;

namespace dev {
    template<class T>
    consteval auto collapse() { return std::declval<T>(); }
}

template<class T>
using collapse_t = decltype(dev::collapse<T>());

}


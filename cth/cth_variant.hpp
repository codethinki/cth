#pragma once

namespace cth::var {
/**
 * \brief std::visit lambda overload capsule
 * \note every std:variant visitation must be specified
 */
template<class... Ts> struct overload : Ts... {
    using Ts::operator()...;
};
template<class... Ts> overload(Ts...)->overload<Ts...>;

/**
 * \brief std::visit single lambda capsule
 */
template<class Ts>
struct visitor : Ts {
    explicit visitor(Ts f) : Ts(f) {}
    using Ts::operator();
};
template<class Ts>
visitor(Ts f)->visitor<Ts>;

}

#pragma once

namespace cth::var {
/**
 * callable with the call operators of all `Ts`
 * @details use it as visitor of `std::visit`
 * @tparam Ts callables to merge
 * @note `std::visit` needs a call operator for every alternative
 */
template<class... Ts>
struct overload : Ts... {
    using Ts::operator()...;
};
template<class... Ts>
overload(Ts...) -> overload<Ts...>;

/**
 * callable that wraps `Ts`
 * @details use it as visitor of `std::visit`
 * @tparam Ts callable to wrap
 */
template<class Ts>
struct visitor : Ts {
    explicit visitor(Ts f) : Ts(f) {}
    using Ts::operator();
};
template<class Ts>
visitor(Ts f) -> visitor<Ts>;

} // namespace cth::var

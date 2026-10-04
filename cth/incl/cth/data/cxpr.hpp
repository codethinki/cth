#pragma once
#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>

namespace cth::dt {

/**
 * copies the range returned by a constexpr generator into a std::array
 * @details allows results computed with constexpr std::vector & co. to outlive constant evaluation
 * @tparam Generator callable without arguments that returns a sized range
 */
template<auto Generator>
[[nodiscard]] consteval auto as_cxpr_array() {
    using value_type = std::ranges::range_value_t<decltype(Generator())>;
    constexpr size_t SIZE = std::ranges::size(Generator());

    std::array<value_type, SIZE> result{};
    std::ranges::copy(Generator(), result.begin());
    return result;
}

}

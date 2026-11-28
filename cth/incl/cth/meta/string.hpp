#pragma once
#include "cth/types.hpp"

#include <cstddef>
#include <algorithm>
#include <array>

namespace cth::mta {
template<uz Size, class CType = char>
class cval_string {
public:
    constexpr cval_string(CType const (&str)[Size]) {
        std::copy_n(str, Size, _val);
    }

private:
    std::array<CType, Size> _val{};

public:
    constexpr bool operator==(cval_string const&) const = default;
};

template<uz Sz1, uz Sz2, class CType>
[[nodiscard]] constexpr bool operator==(cval_string<Sz1, CType> left, cval_string<Sz2, CType> right) {
    if constexpr(Sz1 != Sz2)
        return false;
    else return left == right;
}

template<uz Sz1, uz Sz2, class CType>
[[nodiscard]] constexpr bool operator!=(cval_string<Sz1, CType> left, cval_string<Sz2, CType> right) {
    return !(left == right);
}

template<cval_string Str>
[[nodiscard]] constexpr auto operator""_cval() { return Str; }
}

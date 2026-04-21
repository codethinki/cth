#pragma once
#include <string_view>
#include <array>


namespace cth::mta {
template<size_t Size, class CType = char>
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


template<size_t Sz1, size_t Sz2, class CType>
[[nodiscard]] constexpr bool operator==(cval_string<Sz1, CType> left, cval_string<Sz2, CType> right) {
    if constexpr(Sz1 != Sz2)
        return false;
    else return left == right;
}

template<size_t Sz1, size_t Sz2, class CType>
[[nodiscard]] constexpr bool operator!=(cval_string<Sz1, CType> left, cval_string<Sz2, CType> right) {
    return !(left == right);
}

template<cval_string Str>
[[nodiscard]] constexpr auto operator""_cval() {
    return Str;
}
}

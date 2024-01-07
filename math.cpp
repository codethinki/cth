#include "pch.h"
#include "math.hpp"
#include <string>
namespace cth::math {
template<integral_t T>
[[nodiscard]] constexpr T cycle(const T x, const T a, const T b) {
    assert(b > a && "cycle: a must be the lower value");
    const T off = (x - a) % (b - a);
    return off < 0 ? b + off : a + off;
}

template<floating_point_t T>
[[nodiscard]] constexpr T heronSqrt(const T x, const T precision) {
    assert(x >= 0 && "heron sqrt: x smaller than 0");

    T val = x * static_cast<T>(0.5);
    while(val * val < x - precision || val * val > x + precision) val = (val + x / val) * static_cast<T>(0.5);

    return val;
}

template<floating_point_t T>
[[nodiscard]] constexpr T exp(const T x, const T precision) {
    uint32_t n = 1;
    double result = 1, term = x;
    while(term > precision) {
        result += term;
        ++n;
        term = term * x / n;
    }
    return result;
}

template<integral_t T, unsigned_t U, unsigned_t V>
[[nodiscard]] constexpr T sqam(const T base, const U power, const V mod) {
    assert(mod > 0 && "sqam: mod below 1 not valid");

    auto bitArr = cth::math::bits::toBitArr(power);

    T val = base;
    for(int i = cth::math::bits::firstSetBit(power) + 1; i < bitArr.size(); ++i) {
        val *= val;
        if(bitArr[i]) val *= base;
        val %= mod;
    }
    return val;
}

namespace bits {
    template<integral_t T>
    constexpr array<bool, sizeof(T) * 8> toBitArr(const T val) {
        constexpr uint32_t bitLength = sizeof(T) * 8;
        array<bool, bitLength> bits{};

        for(int i = 0; i < bitLength; i++) bits[i] = (static_cast<T>(1) << bitLength - 1 - i & val) > 0;

        return bits;
    }

    template<integral_t T>
    void printBits(const T val) {
        array<bool, sizeof(T) * 8> arr = bits::toBitArr(val);
        for(uint32_t i = 0; i < sizeof(T) * 8; i++) cout << std::to_string((arr[i] ? 1 : 0));
        cout << ": " << std::to_string(val) << '\n';
    }

    template<unsigned_t T>
    constexpr size_t firstSetBit(const T x) {
        uint32_t pos = x;

        while((x & (static_cast<T>(1) << sizeof(T) * 8 - 1 - pos)) == 0 && pos < sizeof(T) * 8) pos++;

        return pos;
    }
} // namespace bits

} // namespace cth::math


//IMPLEMENT Move these functions to math.hpp to avoid linker errors
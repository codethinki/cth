#pragma once
#include <cassert>
#include <cstdint>
#include <iostream>

#include "concepts.hpp"

namespace cth {
using namespace cth::concepts;
using namespace std;

//------------------
// DECLARATIONS
//------------------
namespace math {
    template<arithmetic_t T>
    [[nodiscard]] constexpr T abs(const T val) { return val < 0 ? -val : val; }

    /**
 * \brief cycles a value between a and b
 * \param a included
 * \param b excluded
 */
    template<integral_t T>
    [[nodiscard]] constexpr T cycle(T x, T a, T b);

    template<floating_point_t T>
    [[nodiscard]] constexpr T heronSqrt(T x, T precision = static_cast<T>(1e-6));

    template<floating_point_t T>
    [[nodiscard]] constexpr T exp(T x, T precision = static_cast<T>(1e-6));

    template<arithmetic_t T>
    [[nodiscard]] constexpr T map(const T x, const T in_a, const T in_b, const T out_a, const T out_b) {
        return out_a + (out_b - out_a) * ((x - in_a) / (in_b - in_a));
    }
    template<arithmetic_t T>
    [[nodiscard]] T dist(const T x1, const T y1, const T x2, const T y2) {
        const T dx = x2 - x1, dy = y2 - y1;
        return static_cast<T>(sqrt(dx * dx + dy * dy));
    }

    /**
     *\param x value
     * \param a lowest
     * \param b highest
     */
    template<arithmetic_t T>
    [[nodiscard]] constexpr bool inRange(const T x, const T a, const T b) {
        assert(a < b && "a must be smaller than b");
        return a <= x && x <= b;
    }
    /**
     *\brief inRange for two values
    */
    template<arithmetic_t T>
    [[nodiscard]] constexpr bool inRange2d(const T x, const T a_x, const T b_x, const T y, const T a_y, const T b_y) {
        return math::inRange(x, a_x, b_x) && math::inRange(y, a_y, b_y);
    }

    /**
     * \brief square and multiply algorithm
     */
    template<integral_t T, unsigned_t U, unsigned_t V>
    [[nodiscard]] constexpr T sqam(T base, U power, V mod);

    namespace bits {
        template<integral_t T>
        constexpr array<bool, sizeof(T) * 8> toBitArr(T val);

        template<integral_t T>
        void printBits(T val);

        template<unsigned_t T>
        constexpr size_t firstSetBit(T x);
    }

} // namespace math

//---------------------
// CONSTEXPR FUNC
//---------------------
namespace expr::math {

    template<arithmetic_t T>
    [[nodiscard]] constexpr T map(const T x, const T in_a, const T in_b, const T out_a, const T out_b) {
        return cth::math::map(x, in_a, in_b, out_a, out_b);
    }

    template<floating_point_t T>
    [[nodiscard]] constexpr T heronSqrt(const T x, const T precision = static_cast<T>(1e-6)) { return cth::math::heronSqrt(x, precision); }

    template<integral_t T>
    [[nodiscard]] constexpr T cycle(const T x, const T a, const T b) { return cth::math::cycle(x, a, b); }

    template<arithmetic_t T>
    [[nodiscard]] constexpr T dist(const T x1, const T y1, const T x2, const T y2) {
        const auto dx = static_cast<double>(x2 - x1), dy = static_cast<double>(y2 - y1);
        return static_cast<T>(heronSqrt(dx * dx + dy * dy));
    }

    template<arithmetic_t T>
    [[nodiscard]] constexpr bool inRange(const T x, const T a, const T b) { return cth::math::inRange(x, a, b); }

    template<arithmetic_t T>
    [[nodiscard]] constexpr bool inRange2d(const T x, const T a_x, const T b_x, const T y, const T a_y, const T b_y) {
        return cth::math::inRange2d(x, a_x, b_x, y, a_y, b_y);
    }

    template<integral_t T, unsigned_t U, unsigned_t V>
    [[nodiscard]] constexpr T sqam(const T base, const U power, const V mod) { return cth::math::sqam(base, power, mod); }

    namespace bits {
        template<unsigned_t T>
        [[nodiscard]] constexpr size_t firstSetBit(const T x) { return cth::math::bits::firstSetBit(x); }

        template<integral_t T>
        constexpr std::array<bool, sizeof(T) * 8> toBitArr(const T val) { return cth::math::bits::toBitArr(val); }
    }

} // namespace expr::math
} // namespace cth

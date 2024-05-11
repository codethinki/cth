#pragma once
#include <array>
#include <cassert>
#include <string>
#include <type_traits>

#include "cth_concepts.hpp"
#include "cth_log.hpp"

namespace cth {
//------------------
// DECLARATIONS
//------------------
namespace num {
    template<type::arithmetic T>
    [[nodiscard]] constexpr T abs(T val);

    /**
 * \brief cycles a value between a and b
 * \param a included
 * \param b excluded
 */
    template<std::integral T>
    [[nodiscard]] constexpr T cycle(T x, T a, T b);

    template<std::floating_point T>
    [[nodiscard]] constexpr T exp(T x, T precision = static_cast<T>(1e-6));

    template<std::floating_point T>
    [[nodiscard]] constexpr T heronSqrt(T x, T precision = static_cast<T>(1e-6));

    template<std::floating_point T>
    [[nodiscard]] constexpr T map(T x, T in_a, T in_b, T out_a, T out_b);

    template<type::arithmetic T>
    [[nodiscard]] T dist(T x1, T y1, T x2, T y2);

    /**
     *\param x value
     * \param a lowest
     * \param b highest
     */
    template<type::arithmetic T>
    [[nodiscard]] constexpr bool inRange(T x, T a, T b);
    /**
     *\brief inRange for two values
    */
    template<type::arithmetic T, type::arithmetic U>
    [[nodiscard]] constexpr bool inRange2d(T x, T a_x, T b_x, U y, U a_y, U b_y);

    /**
     * \brief square and multiply algorithm
     */
    template<std::integral T, std::unsigned_integral U, std::unsigned_integral V>
    [[nodiscard]] constexpr T sqam(T base, U power, V mod);

    namespace bits {
        //no unit tests written
        template<std::integral T>
        void printBits(T val);

        template<std::integral T>
        constexpr std::array<bool, sizeof(T) * 8> toBitArr(T val);

        template<std::unsigned_integral T>
        constexpr size_t firstSetBit(T x);
    } //namespace bits

} // namespace num

//-----------------------
// IMPLEMENTATIONS
//-----------------------
namespace num {
    template<type::arithmetic T>
    [[nodiscard]] constexpr T abs(const T val) { return val < 0 ? -val : val; }

    template<std::integral T>
    [[nodiscard]] constexpr T cycle(const T x, const T a, const T b) {
        CTH_ERR(b <= a, "invalid input: ({0}) a < b ({1}) required", a, b) throw details->exception();
        const T off = (x - a) % (b - a);
        return off < 0 ? b + off : a + off;
    }

    template<std::floating_point T>
    [[nodiscard]] constexpr T exp(const T x, const T precision) {
        uint32_t n = 1;
        T result = 1, term = x;
        while(num::abs(term) > precision) {
            result += term;
            ++n;
            term *= x / static_cast<T>(n);
        }
        return result;
    }

    template<std::floating_point T>
    [[nodiscard]] constexpr T heronSqrt(const T x, const T precision) {

        CTH_ERR(x < 0, "heronSqrt: x ({}) >= 0 required", x) throw details->exception();

        T val = x * static_cast<T>(0.5);
        while(val * val < x - precision || val * val > x + precision) val = (val + x / val) * static_cast<T>(0.5);

        return val;
    }

    template<std::floating_point T>
    [[nodiscard]] constexpr T map(const T x, const T in_a, const T in_b, const T out_a, const T out_b) {
        return out_a + (out_b - out_a) * ((x - in_a) / (in_b - in_a));
    }

    template<type::arithmetic T>
    [[nodiscard]] T dist(const T x1, const T y1, const T x2, const T y2) {
        const T dx = x2 - x1, dy = y2 - y1;
        return static_cast<T>(std::sqrt(dx * dx + dy * dy));
    }

    template<type::arithmetic T>
    [[nodiscard]] constexpr bool inRange(const T x, const T a, const T b) {
        assert(a < b && "invalid input: a <= b required");
        return a <= x && x <= b;
    }

    template<type::arithmetic T, type::arithmetic U>
    [[nodiscard]] constexpr bool inRange2d(const T x, const T a_x, const T b_x, const U y, const U a_y, const U b_y) {
        return num::inRange(x, a_x, b_x) && num::inRange(y, a_y, b_y);
    }

    template<std::integral T, std::unsigned_integral U, std::unsigned_integral V>
    [[nodiscard]] constexpr T sqam(const T base, const U power, const V mod) {
        CTH_ERR(mod < 0, "invalid input: mod ({}) > 0 required", mod) throw details->exception();

        auto bitArr = bits::toBitArr(power);


        T val = base;
        for(auto i = bits::firstSetBit(power) + 1; i < bitArr.size(); ++i) {
            val *= val;
            if(bitArr[i]) val *= base;
            val %= mod;
        }
        return val;
    }

    namespace bits {
        template<std::integral T>
        void printBits(const T val) {
            std::array<bool, sizeof(T) * 8> arr = bits::toBitArr(val);

            std::ranges::for_each(arr, [](const bool b) { std::cout << (b ? '1' : '0'); });
            std::cout << ": " << std::to_string(val) << '\n';
        }

        template<std::integral T>
        constexpr std::array<bool, sizeof(T) * 8> toBitArr(const T val) {
            constexpr uint32_t bitLength = sizeof(T) * 8;
            std::array<bool, bitLength> bits{};

            for(uint32_t i = 0; i < bitLength; i++) bits[i] = ((static_cast<T>(1) << (bitLength - 1 - i)) & val) > 0;

            return bits;
        }

        template<std::unsigned_integral T>
        constexpr size_t firstSetBit(const T x) {
            size_t pos = 0;


            while((x & (static_cast<T>(1) << (sizeof(T) * 8 - 1 - pos))) == 0 && pos < sizeof(T) * 8) ++pos;

            return pos;
        }
    } // namespace bits
} // namespace num

//---------------------
// CONSTEXPR FUNC
//---------------------
namespace expr::num {

    template<type::arithmetic T>
    [[nodiscard]] constexpr T map(const T x, const T in_a, const T in_b, const T out_a, const T out_b) {
        return cth::num::map(x, in_a, in_b, out_a, out_b);
    }

    template<std::floating_point T>
    [[nodiscard]] constexpr T heronSqrt(const T x, const T precision = static_cast<T>(1e-6)) { return cth::num::heronSqrt(x, precision); }

    template<std::integral T>
    [[nodiscard]] constexpr T cycle(const T x, const T a, const T b) { return cth::num::cycle(x, a, b); }

    template<type::arithmetic T>
    [[nodiscard]] constexpr T dist(const T x1, const T y1, const T x2, const T y2, const T precision = static_cast<T>(1e-6)) {
        const auto dx = static_cast<double>(x2 - x1), dy = static_cast<double>(y2 - y1);
        return static_cast<T>(cth::num::heronSqrt(dx * dx + dy * dy, precision));
    }

    template<type::arithmetic T>
    [[nodiscard]] constexpr bool inRange(const T x, const T a, const T b) { return cth::num::inRange(x, a, b); }

    template<type::arithmetic T>
    [[nodiscard]] constexpr bool inRange2d(const T x, const T a_x, const T b_x, const T y, const T a_y, const T b_y) {
        return cth::num::inRange2d(x, a_x, b_x, y, a_y, b_y);
    }

    template<std::integral T, std::unsigned_integral U, std::unsigned_integral V>
    [[nodiscard]] constexpr T sqam(const T base, const U power, const V mod) { return cth::num::sqam(base, power, mod); }

    namespace bits {
        template<std::unsigned_integral T>
        [[nodiscard]] constexpr size_t firstSetBit(const T x) { return cth::num::bits::firstSetBit(x); }

        template<std::integral T>
        constexpr std::array<bool, sizeof(T) * 8> toBitArr(const T val) { return cth::num::bits::toBitArr(val); }
    }

} // namespace expr::num
} // namespace cth

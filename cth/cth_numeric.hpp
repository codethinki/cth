#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <string>


#include "cth_concepts.hpp"
#include "cth_log.hpp"

namespace cth {
using namespace cth::type;
using namespace std;

//------------------
// DECLARATIONS
//------------------
namespace num {
    template<arithmetic_t T>
    [[nodiscard]] constexpr T abs(T val);

    /**
 * \brief cycles a value between a and b
 * \param a included
 * \param b excluded
 */
    template<integral_t T>
    [[nodiscard]] constexpr T cycle(T x, T a, T b);

    template<floating_point_t T>
    [[nodiscard]] constexpr T exp(T x, T precision = static_cast<T>(1e-6));

    template<floating_point_t T>
    [[nodiscard]] constexpr T heronSqrt(T x, T precision = static_cast<T>(1e-6));

    template<floating_point_t T>
    [[nodiscard]] constexpr T map(T x, T in_a, T in_b, T out_a, T out_b);

    template<arithmetic_t T>
    [[nodiscard]] T dist(T x1, T y1, T x2, T y2);

    /**
     *\param x value
     * \param a lowest
     * \param b highest
     */
    template<arithmetic_t T>
    [[nodiscard]] constexpr bool inRange(T x, T a, T b);
    /**
     *\brief inRange for two values
    */
    template<arithmetic_t T, arithmetic_t U>
    [[nodiscard]] constexpr bool inRange2d(T x, T a_x, T b_x, U y, U a_y, U b_y);

    /**
     * \brief square and multiply algorithm
     */
    template<integral_t T, unsigned_t U, unsigned_t V>
    [[nodiscard]] constexpr T sqam(T base, U power, V mod);

    namespace bits {
        //no unit tests written
        template<integral_t T>
        void printBits(T val);

        template<integral_t T>
        constexpr array<bool, sizeof(T) * 8> toBitArr(T val);

        template<unsigned_t T>
        constexpr size_t firstSetBit(T x);
    }

} // namespace num

//-----------------------
// IMPLEMENTATIONS
//-----------------------
namespace num {
    template<arithmetic_t T>
    [[nodiscard]] constexpr T abs(const T val) { return val < 0 ? -val : val; }

    template<integral_t T>
    [[nodiscard]] constexpr T cycle(const T x, const T a, const T b) {
        CTH_LOG(b > a, "a must be the lower value");
        const T off = (x - a) % (b - a);
        return off < 0 ? b + off : a + off;
    }

    template<floating_point_t T>
    [[nodiscard]] constexpr T exp(const T x, const T precision) {
        uint32_t n = 1;
        T result = 1, term = x;
        while(num::abs(term) > precision) {
            result += term;
            ++n;
            term *= x / n;
        }
        return result;
    }

    template<floating_point_t T>
    [[nodiscard]] constexpr T heronSqrt(const T x, const T precision) {

        CTH_ERR((x >= 0), "heronSqrt: x >= 0 required");

        T val = x * static_cast<T>(0.5);
        while(val * val < x - precision || val * val > x + precision) val = (val + x / val) * static_cast<T>(0.5);

        return val;
    }

    template<floating_point_t T>
    [[nodiscard]] constexpr T map(const T x, const T in_a, const T in_b, const T out_a, const T out_b) {
        return out_a + (out_b - out_a) * ((x - in_a) / (in_b - in_a));
    }

    template<arithmetic_t T>
    [[nodiscard]] T dist(const T x1, const T y1, const T x2, const T y2) {
        const T dx = x2 - x1, dy = y2 - y1;
        return static_cast<T>(std::sqrt(dx * dx + dy * dy));
    }

    template<arithmetic_t T>
    [[nodiscard]] constexpr bool inRange(const T x, const T a, const T b) {
        assert(a < b && "a must be smaller than b");
        return a <= x && x <= b;
    }

    template<arithmetic_t T, arithmetic_t U>
    [[nodiscard]] constexpr bool inRange2d(const T x, const T a_x, const T b_x, const U y, const U a_y, const U b_y) {
        return num::inRange(x, a_x, b_x) && num::inRange(y, a_y, b_y);
    }

    template<integral_t T, unsigned_t U, unsigned_t V>
    [[nodiscard]] constexpr T sqam(const T base, const U power, const V mod) {
        assert(mod > 0 && "sqam: mod > 0 required");

        auto bitArr = bits::toBitArr(power);


        T val = base;
        for(int i = bits::firstSetBit(power) + 1; i < bitArr.size(); ++i) {
            val *= val;
            if(bitArr[i]) val *= base;
            val %= mod;
        }
        return val;
    }

    namespace bits {
        template<integral_t T>
        void printBits(const T val) {
            array<bool, sizeof(T) * 8> arr = bits::toBitArr(val);

            ranges::for_each(arr, [](const bool b) { cout << (b ? '1' : '0'); });
            cout << ": " << std::to_string(val) << '\n';
        }

        template<integral_t T>
        constexpr array<bool, sizeof(T) * 8> toBitArr(const T val) {
            constexpr uint32_t bitLength = sizeof(T) * 8;
            array<bool, bitLength> bits{};

            for(int i = 0; i < bitLength; i++) bits[i] = (static_cast<T>(1) << bitLength - 1 - i & val) > 0;

            return bits;
        }

        template<unsigned_t T>
        constexpr size_t firstSetBit(const T x) {
            size_t pos = 0;


            while((x & (static_cast<T>(1) << sizeof(T) * 8 - 1 - pos)) == 0 && pos < sizeof(T) * 8) ++pos;

            return pos;
        }
    } // namespace bits
} // namespace num

//---------------------
// CONSTEXPR FUNC
//---------------------
namespace expr::num {

    template<arithmetic_t T>
    [[nodiscard]] constexpr T map(const T x, const T in_a, const T in_b, const T out_a, const T out_b) {
        return cth::num::map(x, in_a, in_b, out_a, out_b);
    }

    template<floating_point_t T>
    [[nodiscard]] constexpr T heronSqrt(const T x, const T precision = static_cast<T>(1e-6)) { return cth::num::heronSqrt(x, precision); }

    template<integral_t T>
    [[nodiscard]] constexpr T cycle(const T x, const T a, const T b) { return cth::num::cycle(x, a, b); }

    template<arithmetic_t T>
    [[nodiscard]] constexpr T dist(const T x1, const T y1, const T x2, const T y2, const T precision = static_cast<T>(1e-6)) {
        const auto dx = static_cast<double>(x2 - x1), dy = static_cast<double>(y2 - y1);
        return static_cast<T>(cth::num::heronSqrt(dx * dx + dy * dy, precision));
    }

    template<arithmetic_t T>
    [[nodiscard]] constexpr bool inRange(const T x, const T a, const T b) { return cth::num::inRange(x, a, b); }

    template<arithmetic_t T>
    [[nodiscard]] constexpr bool inRange2d(const T x, const T a_x, const T b_x, const T y, const T a_y, const T b_y) {
        return cth::num::inRange2d(x, a_x, b_x, y, a_y, b_y);
    }

    template<integral_t T, unsigned_t U, unsigned_t V>
    [[nodiscard]] constexpr T sqam(const T base, const U power, const V mod) { return cth::num::sqam(base, power, mod); }

    namespace bits {
        template<unsigned_t T>
        [[nodiscard]] constexpr size_t firstSetBit(const T x) { return cth::num::bits::firstSetBit(x); }

        template<integral_t T>
        constexpr std::array<bool, sizeof(T) * 8> toBitArr(const T val) { return cth::num::bits::toBitArr(val); }
    }

} // namespace expr::num
} // namespace cth

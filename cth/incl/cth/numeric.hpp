#pragma once
#include <array>
#include <cassert>
#include <string>

#include "io/log.hpp"


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
    [[nodiscard]] cxpr T cycle(T x, T a, T b);

    template<std::floating_point T>
    [[nodiscard]] cxpr T exp(T x, T precision = static_cast<T>(1e-6));

    template<std::floating_point T>
    [[nodiscard]] cxpr T heronSqrt(T x, T precision = static_cast<T>(1e-6));

    [[nodiscard]] cxpr auto map(auto x, auto in_a, auto in_b, auto out_a, auto out_b);

    template<type::arithmetic T>
    [[nodiscard]] T dist(T x1, T y1, T x2, T y2);

    /**
     * @brief checks if @ref x is in [ @ref a, @ref b)
     */
    [[nodiscard]] cxpr bool in(auto x, auto a, auto b);

    /**
     * @brief in for two values
     */
    template<type::arithmetic T, type::arithmetic U>
    [[nodiscard]] cxpr bool in(T x, T a_x, T b_x, U y, U a_y, U b_y);

    /**
     * \brief square and multiply algorithm
     */
    template<std::integral T, std::unsigned_integral U, std::unsigned_integral V>
    [[nodiscard]] cxpr T sqam(T base, U power, V mod);

    /**
     * @brief calculates the unsigned integral power of an arithmetic base
     * @attention power must be >= 0
     */
    template<type::arithmetic T>
    [[nodiscard]] cxpr T pow(T base, std::integral auto power);


    namespace bits {
        template<std::integral T>
        cxpr std::array<bool, sizeof(T) * 8> toBitArr(T val);

        template<std::unsigned_integral T>
        cxpr size_t firstSetBit(T x);
    } //namespace bits

} // namespace num

//-----------------------
// IMPLEMENTATIONS
//-----------------------
namespace num {
    template<type::arithmetic T>
    [[nodiscard]] cxpr T abs(T val) { return val < 0 ? -val : val; }

    template<std::integral T>
    [[nodiscard]] cxpr T cycle(T x, T a, T b) {
        CTH_CRITICAL(b <= a, "invalid input: ({0}) a < b ({1}) required", a, b) {}
        T const off = (x - a) % (b - a);
        return off < 0 ? b + off : a + off;
    }

    template<std::integral auto Size> requires(Size > 0)
    cval bool power_2() {
        return Size == 0 || ((Size & (Size - 1)) == 0);
    }


    template<std::integral auto Multiple> requires(Multiple > 0)
    cxpr auto align_to(std::integral auto unaligned) {
        if constexpr((Multiple & (Multiple - 1)) == 0) return (unaligned + (Multiple - 1)) & ~(Multiple - 1);

        auto const padding = Multiple - unaligned % Multiple;
        return unaligned + padding;
    }

    template<class T>
    cxpr auto align_to(std::integral auto unaligned) {
        return cth::num::align_to<sizeof(T)>(unaligned);
    }


    template<std::floating_point T>
    [[nodiscard]] cxpr T exp(T x, T precision) {
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
    [[nodiscard]] cxpr T heronSqrt(T x, T precision) {

        CTH_CRITICAL(x < 0, "heronSqrt: x ({}) >= 0 required", x) {}

        T val = x * static_cast<T>(0.5);
        while(val * val < x - precision || val * val > x + precision) val = (val + x / val) * static_cast<T>(0.5);

        return val;
    }

    [[nodiscard]] cxpr auto map(auto x, auto in_a, auto in_b, auto out_a, auto out_b) {
        return out_a + (out_b - out_a) * ((x - in_a) / (in_b - in_a));
    }

    template<type::arithmetic T>
    [[nodiscard]] T dist(T x1, T y1, T x2, T y2) {
        T const dx = x2 - x1, dy = y2 - y1;
        return static_cast<T>(std::sqrt(dx * dx + dy * dy));
    }

    [[nodiscard]] cxpr bool in(auto x, auto a, auto b) {
        assert(a < b && "invalid input: a <= b required");
        return a <= x && x < b;
    }

    [[nodiscard]] cxpr bool in_inc(auto x, auto a, auto b) {
        return a <= x && x <= b;
    }


    template<type::arithmetic T, type::arithmetic U>
    [[nodiscard]] cxpr bool in(T x, T a_x, T b_x, U y, U a_y, U b_y) { return num::in(x, a_x, b_x) && num::in(y, a_y, b_y); }

    template<std::integral T, std::unsigned_integral U, std::unsigned_integral V>
    [[nodiscard]] cxpr T sqam(T base, U power, V mod) {
        CTH_CRITICAL(mod < 0, "invalid input: mod ({}) > 0 required", mod) {}

        auto bitArr = bits::toBitArr(power);


        T val = base;
        for(auto i = bits::firstSetBit(power) + 1; i < bitArr.size(); ++i) {
            val *= val;
            if(bitArr[i]) val *= base;
            val %= mod;
        }
        return val;
    }

    template<type::arithmetic T>
    [[nodiscard]] cxpr T pow(T base, std::integral auto power) {
        CTH_CRITICAL(power < 0, "power must be > 0") {}
        T result{1};
        while(power > 0) {
            if(power % 2 == 1) result *= base;
            base *= base;
            power /= 2;
        }
        return result;
    }


    namespace bits {
        template<std::integral T>
        cxpr std::array<bool, sizeof(T) * 8> toBitArr(T val) {
            cxpr static auto BIT_LENGTH = sizeof(T) * 8;
            std::array<bool, BIT_LENGTH> bits{};

            for(uint32_t i = 0; i < BIT_LENGTH; i++) bits[i] = ((static_cast<T>(1) << (BIT_LENGTH - 1 - i)) & val) > 0;

            return bits;
        }

        template<std::unsigned_integral T>
        cxpr size_t firstSetBit(T x) {
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

    [[nodiscard]] constexpr auto map(auto x, auto in_a, auto in_b, auto out_a, auto out_b) { return cth::num::map(x, in_a, in_b, out_a, out_b); }

    template<std::floating_point T>
    [[nodiscard]] cxpr T heronSqrt(T x, T const precision = static_cast<T>(1e-6)) { return cth::num::heronSqrt(x, precision); }

    template<std::integral T>
    [[nodiscard]] cxpr T cycle(T x, T a, T b) { return cth::num::cycle(x, a, b); }

    template<type::arithmetic T>
    [[nodiscard]] cxpr T dist(T x1, T y1, T x2, T y2, T const precision = static_cast<T>(1e-6)) {
        auto const dx = static_cast<double>(x2 - x1), dy = static_cast<double>(y2 - y1);
        return static_cast<T>(cth::num::heronSqrt(dx * dx + dy * dy, precision));
    }

    [[nodiscard]] cxpr bool in_inc(auto x, auto a, auto b) { return cth::num::in_inc(x, a, b); }
    [[nodiscard]] cxpr bool in(auto x, auto a, auto b) { return cth::num::in(x, a, b); }

    template<type::arithmetic T>
    [[nodiscard]] constexpr bool in(T x, T a_x, T b_x, T y, T a_y, T b_y) { return cth::num::in(x, a_x, b_x, y, a_y, b_y); }

    template<std::integral T, std::unsigned_integral U, std::unsigned_integral V>
    [[nodiscard]] cxpr T sqam(T base, U power, V mod) { return cth::num::sqam(base, power, mod); }


    template<type::arithmetic T>
    [[nodiscard]] cxpr T pow(T base, std::unsigned_integral auto power) { return cth::num::pow(base, power); }


    namespace bits {
        template<std::unsigned_integral T>
        [[nodiscard]] cxpr size_t firstSetBit(T x) { return cth::num::bits::firstSetBit(x); }

        template<std::integral T>
        cxpr std::array<bool, sizeof(T) * 8> toBitArr(T val) { return cth::num::bits::toBitArr(val); }
    }

} // namespace expr::num
} // namespace cth

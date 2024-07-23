#include "../cth/cth_variant.hpp"

#include <gtest/gtest.h>

namespace cth::var {



TEST(overload, main) {
    using variant_t = std::variant<int, float, double>;
    constexpr variant_t x = 42;
    constexpr variant_t y = 42.0;
    constexpr variant_t z = 42.f;

    auto overloadTest = [](variant_t const& v) {
        return std::visit(cth::var::overload{
            [](int) { return "int"; },
            [](float) { return "float"; },
            [](double) { return "double"; }
        }, v);
    };
    EXPECT_EQ(overloadTest(x), "int");
    EXPECT_EQ(overloadTest(y), "double");
    EXPECT_EQ(overloadTest(z), "float");
}
TEST(visitor, main) {
    using variant_t = std::variant<int, float, double>;
    constexpr variant_t x = 42;
    constexpr variant_t y = 42.0;
    constexpr variant_t z = 42.f;
    auto visitorTest = [](variant_t const& v) {
        return std::visit(cth::var::visitor{
            []<typename T>(T const& var) {
                if constexpr(std::is_same_v<std::remove_cv_t<T>, int>) return "int";
                if constexpr(std::is_same_v<std::remove_cv_t<T>, float>) return "float";
                if constexpr(std::is_same_v<std::remove_cv_t<T>, double>) return "double";
            }
        }, v);
    };

    EXPECT_EQ(visitorTest(x), "int");
    EXPECT_EQ(visitorTest(y), "double");
    EXPECT_EQ(visitorTest(z), "float");
}
} // namespace cth

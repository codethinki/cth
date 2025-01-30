// ReSharper disable CppMemberFunctionMayBeStatic
#include "cth/functional.hpp"

#include "cth/test.hpp"

#define FN_TEST(suite, test_name) CTH_EX_TEST(_fn, suite, test_name)


namespace cth::fn {
struct A {
    void voidFn() const {}
    [[nodiscard]] int intFnIntInt(int a, int b) const { return a + b; }
    int IntFnIntInt2(int a, int b) {
        x = a + b;
        return x;
    }

    int x;
};

FN_TEST(to_lambda, member_function) {
    A a{};
    A const& ca = std::as_const(a);


    auto voidLb = to_lambda<&A::voidFn>(a);
    auto intLbIntInt = to_lambda<&A::intFnIntInt>(ca);
    auto intLbIntInt2 = to_lambda<&A::IntFnIntInt2>(a);

    voidLb();


    EXPECT_EQ(intLbIntInt(10, 20), 30);
    EXPECT_EQ(intLbIntInt2(10, 20), 30);
}
}

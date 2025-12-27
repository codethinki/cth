#include "cth/meta/enums.hpp"

#include "test.hpp"

namespace cth::types {
enum class A : size_t {
    AND_RESULT = 0,
    FIRST = 1,
    SECOND = 2,
    OR_RESULT = 3
};
CTH_GEN_ENUM_OVERLOADS(A)

TYPE_TEST(MACRO_CTH_ENUM_OVERLOADS, main) {
    constexpr auto x = A::FIRST;
    constexpr auto y = A::SECOND;

    EXPECT_EQ(x | y, A::OR_RESULT);
    EXPECT_EQ(x & y, A::AND_RESULT);


    auto orEq = x;
    orEq |= y;
    EXPECT_EQ(orEq, A::OR_RESULT);

    auto andEq = x;
    andEq &= y;
    EXPECT_EQ(andEq, A::AND_RESULT);


    EXPECT_TRUE((std::same_as<decltype(*x), std::underlying_type_t<A>>));
    EXPECT_EQ(*x, static_cast<std::underlying_type_t<A>>(x));
}
}

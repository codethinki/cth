#include "test.hpp"

#include "cth/types/typ_tuple.hpp"


namespace cth::type {
    TYPE_TEST(n_tuple, main) {
        [[maybe_unused]] constexpr auto ex1 = n_tuple<5, int>(1);
        [[maybe_unused]] constexpr auto ex2 = n_tuple<5, int>();
        [[maybe_unused]] constexpr auto ex3 = n_tuple<5>(0);

        constexpr auto expected = std::tuple{0, 0, 0, 0, 0};
        constexpr auto result = n_tuple<5, int>();

        ASSERT_EQ(result, expected);
    }

}
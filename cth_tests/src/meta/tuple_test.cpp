#include "test.hpp"

#include "cth/meta/tuple.hpp"


namespace cth::mta {
TYPE_TEST(n_tuple, main) {
    [[maybe_unused]] constexpr auto ex1 = n_tuple<5, int>(1);
    [[maybe_unused]] constexpr auto ex2 = n_tuple<5, int>();
    [[maybe_unused]] constexpr auto ex3 = n_tuple<5>(0);

    constexpr auto expected = std::tuple{0, 0, 0, 0, 0};
    constexpr auto result = n_tuple<5, int>();

    ASSERT_EQ(result, expected);
}

class z {};
class d {};
using a = int;
using b = d;
struct base {};

template<class... Ts>
struct dummy_target_trait {};

TYPE_TEST(trait_apply_t, main) {
    // Applying a tuple of types to a trait
    using tuple_type = std::tuple<a, b, z, base>;

    using expected = dummy_target_trait<a, b, z, base>;
    using result = trait_apply_t<dummy_target_trait, tuple_type>;

    static_assert(std::same_as<result, expected>);
}

TYPE_TEST(trait_apply_t, empty_tuple) {
    // Applying an empty tuple should result in an empty trait pack
    using expected = dummy_target_trait<>;
    using result = trait_apply_t<dummy_target_trait, std::tuple<>>;

    static_assert(std::same_as<result, expected>);
}
}

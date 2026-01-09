#include "../test.hpp"

#include "cth/meta/traits/trait_packs.hpp"

#define PACK_TEST(suite, name) TYPE_TEST(suite, name)


namespace cth::mta {
using illegal_pack = trait_pack<std::ranges::range_value_t, int>;
using pack1 = trait_pack<std::remove_cv_t, double* const>;
using pack1_t = double*;

using pack2 = trait_pack<std::remove_reference_t, bool const&>;
using pack2_t = bool const;

PACK_TEST(unpack_trait, main) {
    static_assert(std::same_as<unpack_trait_t<pack1>, pack1_t>);
    static_assert(std::same_as<unpack_trait_t<pack2>, pack2_t>);
}

PACK_TEST(first_trait_pack, main) {
    static_assert(std::same_as<first_pack_t<pack1>, pack1_t>);

    static_assert(std::same_as<first_pack_t<pack1, pack2>, pack1_t>);
    static_assert(
        std::same_as<
            first_pack_t<illegal_pack, illegal_pack, pack1>,
            pack1_t
        >
    );
}

PACK_TEST(conditional_trait_pack, main) {
    static_assert(std::same_as<conditional_pack_t<true, pack1, pack2>, pack1_t>);
    static_assert(std::same_as<conditional_pack_t<false, pack1, pack2>, pack2_t>);
    static_assert(
        std::same_as<
            conditional_pack_t<true, pack1, illegal_pack>,
            pack1_t
        >
    );
}
}

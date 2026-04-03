#include "test.hpp"

#include "cth/meta/variadic.hpp"
#include "cth/meta/debug.hpp"

namespace cth::mta {
class z {};
class d {};
using a = int;
using b = d;

TYPE_TEST(opt_any_of_t, main) {
    using c = collapse_t<mta::opt_any_of_t<z, a, b, b, b, a, z>>;

    static_assert(std::same_as<c, a>);
}

TYPE_TEST(fallback_convert_to_any_t, main) {
    using c = mta::collapse_t<mta::fallback_convert_to_any_t<z, a, b>>;
    using d = mta::collapse_t<mta::fallback_convert_to_any_t<z, a, b, b, a>>;

    static_assert(std::same_as<c, z>);
    static_assert(std::same_as<d, a>);
}

}

namespace cth::mta {
struct base {};
struct derived : base {};

struct convertible_to_a {
    operator a() const { return 0; }
};

struct convertible_to_z {
    operator z() const { return z{}; }
};

TYPE_TEST(resolve_overload_from_t, exact_match) {
    // T exactly matches one of the options
    using res_a = resolve_overload_from_t<a, z, b, a>;
    using res_z = resolve_overload_from_t<z, a, b, z>;
    using res_b = resolve_overload_from_t<b, a, b, z>;

    static_assert(std::same_as<res_a, a>);
    static_assert(std::same_as<res_z, z>);
    static_assert(std::same_as<res_b, b>);
}

TYPE_TEST(resolve_overload_from_t, standard_promotion) {
    // short promotes to int (a)
    using res_int = resolve_overload_from_t<short, z, b, a>;

    // float promotes to double
    using res_double = resolve_overload_from_t<float, z, a, double>;

    static_assert(std::same_as<res_int, a>);
    static_assert(std::same_as<res_double, double>);
}

TYPE_TEST(resolve_overload_from_t, inheritance_conversion) {
    // derived implicitly converts to base
    using res_base = resolve_overload_from_t<derived, z, a, base>;

    static_assert(std::same_as<res_base, base>);
}

TYPE_TEST(resolve_overload_from_t, user_defined_conversion) {
    // Custom structs implicitly converting to a and z
    using res_conv_a = resolve_overload_from_t<convertible_to_a, z, b, a>;
    using res_conv_z = resolve_overload_from_t<convertible_to_z, z, b, a>;

    static_assert(std::same_as<res_conv_a, a>);
    static_assert(std::same_as<res_conv_z, z>);
}

TYPE_TEST(resolve_overload_from_t, exact_beats_conversion) {
    // If both exact and convertible are present, exact must win
    using res_exact_derived = resolve_overload_from_t<derived, z, base, derived>;
    using res_exact_a = resolve_overload_from_t<convertible_to_a, a, convertible_to_a>;

    static_assert(std::same_as<res_exact_derived, derived>);
    static_assert(std::same_as<res_exact_a, convertible_to_a>);
}

}

namespace cth::mta {
struct ambig {
    operator a() const { return 0; }
    operator z() const { return z{}; }
};
TYPE_TEST(resolves_to_any_of, evaluation) {
    // Valid: Exact match, standard promotion, and upcasting
    static_assert(resolves_to_any_of<a, z, b, a>);
    static_assert(resolves_to_any_of<short, z, b, a>);
    static_assert(resolves_to_any_of<derived, z, a, base>);

    static_assert(!resolves_to_any_of<int, int, int>);

    // Invalid: Unrelated types, empty packs, and ambiguous conversions
    static_assert(!resolves_to_any_of<z, a, b>);
    static_assert(!resolves_to_any_of<a>);
    static_assert(!resolves_to_any_of<ambig, a, z>);
}

TYPE_TEST(tuple_set, default) {
    static_assert(std::same_as<std::tuple<>, unique_tuple_t<>>, "empty");
    static_assert(std::same_as<std::tuple<int>, unique_tuple_t<int>>, "identity");
    static_assert(std::same_as<std::tuple<int, double>, unique_tuple_t<int, int, double, int, double>>);
}
}

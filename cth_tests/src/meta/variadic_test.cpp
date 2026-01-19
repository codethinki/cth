#include "test.hpp"

#include "cth/meta/variadic.hpp"
#include "cth/meta/debug.hpp"

namespace cth::mta {
class z {};
class d{};
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

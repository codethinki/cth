#include "cth/hash.hpp"

#include "cth/test.hpp"

#include <set>

#define HASH_TEST(suite, name) CTH_EX_TEST(_hash, suite, name)


namespace asdf {
struct X {
    int a;
    int b;
};

struct Y {
    X x;
    int a;
};

size_t hash_x(X x) { return x.a ^ x.b; }
}

CTH_HASH_OVERLOAD(asdf::X, asdf::hash_x)
CTH_HASH_AGGREGATE(asdf::Y)

namespace cth::hash {
HASH_TEST(macro_CTH_HASH_OVERLOAD, main) {
    using T = asdf::X;

    T const example{1234, 5678};

    ASSERT_EQ(std::hash<T>{}(example), asdf::hash_x(example));
}

HASH_TEST(macro_CTH_HASH_AGGREGATE, main) {
    using T = asdf::Y;

    T const example{{1234, 5678}, 9};

    [[maybe_unused]] auto val = std::hash<T>{}(example);
}


}

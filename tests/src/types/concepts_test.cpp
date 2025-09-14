#include "test.hpp"

#include "cth/types/typ_concepts.hpp"


namespace cth::type {
TYPE_TEST(satisfies, main) {
    ASSERT_TRUE((satisfies<int, CPT(arithmetic)>));
    ASSERT_TRUE((satisfies<float, CPT(arithmetic)>));
    ASSERT_TRUE((satisfies<double, CPT(arithmetic)>));
}

TYPE_TEST(all_satisfy, main) {
    ASSERT_TRUE((all_satisfy<CPT(arithmetic), int, float, double>));
    ASSERT_FALSE((all_satisfy<CPT(arithmetic), int, float, std::string>));
}

TYPE_TEST(any_satisfy, main) {
    ASSERT_TRUE((any_satisfy<CPT(arithmetic), int, float, double>));
    ASSERT_TRUE((any_satisfy<CPT(arithmetic), int, float, std::string>));
    ASSERT_FALSE((any_satisfy<CPT(arithmetic), std::string, std::vector<int>>));
}
}

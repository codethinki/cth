#include "test.hpp"

#include "cth/meta/concepts.hpp"


namespace cth::mta {
TYPE_TEST(satisfies, main) {
    ASSERT_TRUE((satisfies<int, CPT(arithmetic)>) );
    ASSERT_TRUE((satisfies<float, CPT(arithmetic)>) );
    ASSERT_TRUE((satisfies<double, CPT(arithmetic)>) );
}

TYPE_TEST(all_satisfy, main) {
    ASSERT_TRUE((all_satisfy<CPT(arithmetic), int, float, double>) );
    ASSERT_FALSE((all_satisfy<CPT(arithmetic), int, float, std::string>) );
}

TYPE_TEST(any_satisfy, main) {
    ASSERT_TRUE((any_satisfy<CPT(arithmetic), int, float, double>) );
    ASSERT_TRUE((any_satisfy<CPT(arithmetic), int, float, std::string>) );
    ASSERT_FALSE((any_satisfy<CPT(arithmetic), std::string, std::vector<int>>) );
}

constexpr double nothrow_f(float) noexcept { return 0; }

double f(float) { return 0; }

TYPE_TEST(callable_with, main) {
    ASSERT_TRUE((callable_with<f, float>) );
    ASSERT_FALSE((callable_with<f, std::string>) );
}

TYPE_TEST(call_signature, main) {
    ASSERT_TRUE((call_signature<f, double, float>) );

    ASSERT_FALSE((call_signature<f, double, std::string>) );
}

TYPE_TEST(nothrow_callable_with, main) {
    ASSERT_TRUE((nothrow_callable_with<nothrow_f, float>) );
    ASSERT_FALSE((nothrow_callable_with<f, float>) );
}

TYPE_TEST(nothrow_call_signature, main) {
    ASSERT_TRUE((nothrow_call_signature<nothrow_f, double, float>) );
    ASSERT_FALSE((nothrow_call_signature<nothrow_f, double, std::string>) );
    ASSERT_FALSE((nothrow_call_signature<f, double, float>) );
}

}

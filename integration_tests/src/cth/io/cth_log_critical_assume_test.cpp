#define CTH_CRITICAL_MODE CTH_CRITICAL_MODE_ASSUME
#include "cth_log_critical_test.hpp"

#define LOG_TEST(suite, test_name) CTH_EX_TEST(_log_assume, suite, test_name)


namespace cth::log {

LOG_TEST(critical, constexpr) {
    static_assert(critical(false) == 1);
    static_assert(asserted(true) == 1);
}

LOG_TEST(critical, satisfied_skips_the_statement) {
    EXPECT_EQ(critical(false), 1);
    EXPECT_EQ(asserted(true), 1);
    EXPECT_FALSE(bodyRan);
}

}

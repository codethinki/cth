#define CTH_CRITICAL_MODE CTH_CRITICAL_MODE_ENFORCE
#include "cth_log_critical_test.hpp"

#define LOG_TEST(suite, test_name) CTH_EX_TEST(_log_enforce, suite, test_name)


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

LOG_TEST(critical, violated_terminates_without_statement_and_log) {
    ASSERT_DEATH({ report_on_terminate(); critical(true); }, "body: false, log: false");
}

LOG_TEST(assert, violated_terminates_without_statement_and_log) {
    ASSERT_DEATH({ report_on_terminate(); asserted(false); }, "body: false, log: false");
}

}

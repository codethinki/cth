#define CTH_CRITICAL_MODE CTH_CRITICAL_MODE_DEBUG
#include "cth_log_critical_test.hpp"

#define LOG_TEST(suite, test_name) CTH_EX_TEST(_log_debug, suite, test_name)


namespace cth::log {

LOG_TEST(critical, satisfied_skips_the_statement) {
    EXPECT_EQ(critical(false), 1);
    EXPECT_EQ(asserted(true), 1);
    EXPECT_FALSE(bodyRan);
}

LOG_TEST(critical, violated_runs_the_statement_logs_and_terminates) {
    ASSERT_DEATH({ report_on_terminate(); critical(true); }, "body: true, log: true");
}

LOG_TEST(assert, violated_runs_the_statement_logs_and_terminates) {
    ASSERT_DEATH({ report_on_terminate(); asserted(false); }, "body: true, log: true");
}

}

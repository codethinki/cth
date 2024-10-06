#include "../../cth/io/log.hpp"

#include <gtest/gtest.h>


namespace cth::log {


TEST(log_macros, debug) {
#ifdef CTH_DEBUG_MODE
    int x = 0;

    CTH_LOG(true, "CTH_LOG {}", x) ++x;
    ASSERT_EQ(x, 1);

    CTH_INFORM(true, "CTH_INFO {}", x) ++x;
    ASSERT_EQ(x, 2);

    CTH_WARN(true, "CTH_WARN {}", x) ++x;
    ASSERT_EQ(x, 3);
    try {
        CTH_ERR(true, "CTH_ERR with detail \"hello {}\" ", x) {
            details->add("hello {}", x);
            throw except::data_exception{1, details->exception()};

        }
        FAIL() << "CTH_ERR must throw here";
    }
    catch([[maybe_unused]] except::default_exception& e) { ASSERT_TRUE(true); }
#endif
    ASSERT_EQ(1, 1);

}
TEST(log_macros, stable) {
    int x = 0;


    CTH_STABLE_LOG(true, "stable log {}", x) ++x;
    ASSERT_EQ(x, 1);

    CTH_STABLE_INFORM(true, "stable info {}", x) ++x;
    ASSERT_EQ(x, 2);

    CTH_STABLE_WARN(true, "stable warning {}", x) ++x;
    ASSERT_EQ(x, 3);


    try {
        CTH_STABLE_ERR(true, "stable error {}", x) {
            ++x;
            throw details->exception();
        }
        FAIL() << "CTH_STABLE_ERR must throw here";
    }
    catch([[maybe_unused]] cth::except::default_exception const&) { ASSERT_TRUE(true); }
    ASSERT_EQ(x, 4);
}



} // namespace cth

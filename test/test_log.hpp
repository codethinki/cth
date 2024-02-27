#pragma once
#include <gtest/gtest.h>
#include "../cth/cth_log.hpp"

namespace cth {


TEST(headerLog, LogMacros) {
#ifdef _DEBUG
    int x = 0;

    CTH_LOG(false, "log") x = 1;
    ASSERT_EQ(x, 1);

    CTH_INFORM(false, "hint") x = 2;
    ASSERT_EQ(x, 2);

    CTH_WARN(false, "hint") x = 3;
    ASSERT_EQ(x, 3);
    try {
        CTH_ERR(false, "test") {
            details->add("hello");
            throw except::data_exception{1, details->exception()};
        }
    }
    catch(except::default_exception& e) {
    }
#endif
    ASSERT_EQ(1, 1);

}
TEST(headerLog, StableLogMacros) {

    int x = 0;

    CTH_STABLE_LOG(false, "log") x = 1;
    ASSERT_EQ(x, 1);

    CTH_STABLE_INFORM(false, "hint") x = 2;
    ASSERT_EQ(x, 2);

    CTH_STABLE_WARN(false, "warn") x = 3;
    ASSERT_EQ(x, 3);

    CTH_STABLE_ERR(false, "warn") x = 4;
    ASSERT_EQ(x, 4);
}

} // namespace cth

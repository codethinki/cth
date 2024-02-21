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

    try {
        CTH_WARN(false, "test throw") throw details->exception();
    }
    catch(cth::except::default_exception<except::WARNING>& e) {
        out::console.println(e.string());
    }
#endif
    ASSERT_EQ(1, 1);

}
TEST(headerLog, StableLogMacros) {
#ifdef _DEBUG
    int x = 0;

    CTH_STABLE_LOG(false, "log") x = 1;
    ASSERT_EQ(x, 1);

    CTH_STABLE_INFORM(false, "hint") x = 2;
    ASSERT_EQ(x, 2);

    CTH_STABLE_WARN(false, "warn") x = 3;
    ASSERT_EQ(x, 3);
#endif
}

} // namespace cth

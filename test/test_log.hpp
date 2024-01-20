#pragma once
#include <gtest/gtest.h>

#include "../cth/src/cth_log.hpp"

namespace cth {


    TEST(headerLog, LogMacros) {
#ifdef _DEBUG

        int x = 0;
        CTH_LOG(false && "log") x = 1;
        ASSERT_EQ(x, 1);

        CTH_HINT(false && "hint") x = 2;
        ASSERT_EQ(x, 2);

        CTH_WARN(false && "warn") x = 3;
        ASSERT_EQ(x, 3);
#endif
        ASSERT_EQ(1, 1);

    }

}

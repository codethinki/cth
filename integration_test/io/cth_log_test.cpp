#include "../../cth/io/cth_log.hpp"

#include <gtest/gtest.h>

#define FMT_ARGS(message, ...) std::format(message, __VA_ARGS__);

namespace cth::log {


TEST(log_macros, debug) {
#ifdef _DEBUG
    int x;

    CTH_LOG(true, "log") x = 1;
    ASSERT_EQ(x, 1);

    CTH_INFORM(true, "info") x = 2;
    ASSERT_EQ(x, 2);

    CTH_WARN(true, "warning") x = 3;
    ASSERT_EQ(x, 3);
    try {
        CTH_ERR(true, "err with description") {
            details->add("hello");
            throw except::data_exception{1, details->exception()};
        }
    }
    catch(except::default_exception& e) { std::print("{}", e.what()); }
#endif
    ASSERT_EQ(1, 1);

}
TEST(log_macros, stable) {
    int x = 0;


    CTH_STABLE_LOG(true, "stable log {}", x) x = 1;
    ASSERT_EQ(x, 1);

    CTH_STABLE_INFORM(true, "stable info {0}, {1}", x, 2) x = 2;
    ASSERT_EQ(x, 2);

    CTH_STABLE_WARN(true, "stable warning") x = 3;
    ASSERT_EQ(x, 3);

    CTH_STABLE_ERR(true, "stable error") x = 4;
    ASSERT_EQ(x, 4);
}

} // namespace cth

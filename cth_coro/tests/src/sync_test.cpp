#include "test.hpp"


#include "cth/coro/sync.hpp"

namespace cth::co {

CORO_TEST(sync, return_void) {
    bool ran = false;

    auto task = [&](bool& r) -> sync_task<void> {
        r = true;
        co_return;
    };

    sync(task(ran));
    EXPECT_TRUE(ran);
}

CORO_TEST(sync, return_value) {
    auto task = []() -> sync_task<int> { co_return 42; };

    int const result = sync(task());
    EXPECT_EQ(result, 42);
}

CORO_TEST(sync, exception_thrown) {
    auto task = []() -> sync_task<void> {
        throw std::runtime_error("test exception");
        co_return;
    };

    EXPECT_THROW(sync(task()), std::runtime_error);
}

CORO_TEST(sync, exception_with_value) {
    auto task = []() -> sync_task<int> {
        throw std::logic_error("test logic error");
        co_return 0;
    };

    EXPECT_THROW(sync(task()), std::logic_error);
}

}

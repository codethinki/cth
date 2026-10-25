#include "cth/coro/awaiters/wait_awaiter.hpp"

#include "../test.hpp"

#include "cth/coro/awaiters/data_awaiter.hpp"
#include "cth/coro/scheduler.hpp"
#include "cth/meta/coro.hpp"

#include <memory>

#define AWAITER_TEST(suite, name) CORO_EX_TEST(_awaiters, suite, name)


namespace cth::co {

auto const now() { return std::chrono::steady_clock::now(); }

scheduler scheduler{};

AWAITER_TEST(wait_awaiter, main) {
    wait_awaiter awaiter{now(), scheduler};

    static_assert(awaitable<wait_awaiter>, "must be awaitable");

    EXPECT_TRUE(awaiter.await_ready());

    awaiter.timePoint += std::chrono::days{1};

    EXPECT_FALSE(awaiter.await_ready());

    awaiter.await_resume();
}

AWAITER_TEST(data_awaiter, moves_the_value_out) {
    data_awaiter awaiter{std::make_unique<int>(3)};

    EXPECT_TRUE(awaiter.await_ready());

    auto const value = awaiter.await_resume();
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 3);
}

AWAITER_TEST(data_awaiter, keeps_a_reference) {
    int value = 3;
    data_awaiter<int&> awaiter{value};

    EXPECT_EQ(&awaiter.await_resume(), &value);
}


}

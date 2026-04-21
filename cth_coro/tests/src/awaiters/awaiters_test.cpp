#include "cth/coro/awaiters/wait_awaiter.hpp"

#include "../test.hpp"

#include "cth/meta/coro.hpp"

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


}

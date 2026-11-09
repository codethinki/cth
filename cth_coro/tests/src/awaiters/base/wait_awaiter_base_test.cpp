#include "cth/coro/awaiters/base/wait_awaiter_base.hpp"

#include "../../test.hpp"

#include <cth/coro/awaiters/base/native_handle_awaiter_base.hpp>
#include <cth/coro/exception.hpp>
#include <cth/coro/executor.hpp>
#include <cth/coro/scheduler.hpp>
#include <cth/coro/strand.hpp>
#include <cth/coro/tasks/executor_task.hpp>
#include <cth/os/osdef.hpp>

#include <coroutine>
#include <cstdio>
#include <memory>
#include <system_error>

namespace cth::co {
namespace {
    // skips the ready check, the wait of the context always runs
    struct forced_wait_awaiter : native_handle_awaiter_base {
        using native_handle_awaiter_base::native_handle_awaiter_base;

        [[nodiscard]] static constexpr bool await_ready() noexcept { return false; }
    };

    struct file_closer {
        void operator()(std::FILE* file) const noexcept { std::fclose(file); }
    };

    /**
     * gets the error code through the callback of `wait_awaiter_base::resume()`
     */
    struct failed_wait_awaiter : wait_awaiter_base {
        explicit failed_wait_awaiter(std::error_code code) { resume(std::noop_coroutine())(code); }
    };

    auto await_failing(os::native_handle_t handle) -> executor_task<bool> {
        bool caught = false;
        try {
            co_await forced_wait_awaiter{handle};
        } catch(except::coro_exception const&) {
            caught = true;
        }
        co_return caught;
    }
}

CORO_TEST(wait_awaiter_base, failed_wait_throws_its_code) {
    auto const code = std::make_error_code(std::errc::operation_canceled);
    failed_wait_awaiter const awaiter{code};

    try {
        awaiter.await_resume();
        FAIL() << "the failed wait must throw";
    } catch(except::coro_exception const& e) {
        EXPECT_EQ(e.code(), code);
    }
}

CORO_TEST(wait_awaiter_base, reached_wait_does_not_throw) {
    failed_wait_awaiter const awaiter{std::error_code{}};

    EXPECT_NO_THROW(awaiter.await_resume());
}

CORO_TEST(coro_exception, code_is_empty_without_a_wait_error) {
    except::coro_exception const e{"message"};

    EXPECT_FALSE(e.code());
}

CORO_TEST(coro_exception, add_stores_the_code) {
    auto const code = std::make_error_code(std::errc::timed_out);
    except::coro_exception e{"message"};

    e.add(code);

    EXPECT_EQ(e.code(), code);
}

// epoll does not support regular files: the wait on one fails with operation_not_supported
CORO_TEST(wait_awaiter_base, failed_wait_throws_in_caller) {
#ifdef CTH_FS_POSIX
    std::unique_ptr<std::FILE, file_closer> const file{std::tmpfile()};
    ASSERT_NE(file, nullptr);
    os::native_handle_t const handle = fileno(file.get());

    scheduler sched{autostart, 1};
    strand serial{sched};

    EXPECT_TRUE(executor{sched}.sync_wait(await_failing(handle)));
    EXPECT_TRUE(executor{serial}.sync_wait(await_failing(handle)));
#else
    GTEST_SKIP() << "needs a handle that the wait rejects";
#endif
}

}

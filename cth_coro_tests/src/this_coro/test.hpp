#pragma once

#include "../test.hpp"
#include "cth/coro/tasks/sync_task.hpp"

#include <cth/coro/executor.hpp>

#define THIS_CORO_EX_TEST(ex, suite, name) CORO_EX_TEST(_this_coro##ex, suite, name)

#define THIS_CORO_TEST(suite, name) THIS_CORO_EX_TEST(, suite, name)

namespace cth::co {

template<class Task>
auto sync_wait(executor& exec, Task task) {
    using value_type = typename Task::promise_type::value_type;

    auto wrapper = [](executor& exec, Task t) -> sync_task<value_type> {
        if constexpr(std::is_void_v<value_type>) { co_await exec.spawn(std::move(t)); } else {
            co_return co_await exec.spawn(std::move(t));
        }
    };

    auto wrapped = wrapper(exec, std::move(task));
    wrapped.handle().resume();

    wrapped.wait();

    if constexpr(!type::is_void<value_type>)
        return std::move(*wrapped.handle().promise().result);
}

}
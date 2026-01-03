#pragma once

#include "../test.hpp"
#include "cth/coro/tasks/sync_task.hpp"
#include "cth/coro/executor.hpp"

#define THIS_CORO_EX_TEST(ex, suite, name) CORO_EX_TEST(_this_coro##ex, suite, name)

#define THIS_CORO_TEST(suite, name) THIS_CORO_EX_TEST(, suite, name)

namespace cth::co {

template<class X, class A, class B>
concept same_v2 = std::same_as<A, B>;

template<class Task>
awaited_t<Task> sync_wait(executor& exec, Task task) {
    using value_type = typename Task::promise_type::value_type;

    auto wrapper = [](executor& exec, Task t) -> sync_task<value_type> {
        if constexpr(type::is_void<value_type>)
            co_await exec.spawn(std::move(t));
        else
            co_return co_await exec.spawn(std::move(t));
    };

    auto wrapped = wrapper(exec, std::move(task));
    static_assert(std::same_as<typename decltype(wrapped)::promise_type::value_type, value_type>);

    wrapped.handle().resume();

    wrapped.wait();

    if constexpr(!type::is_void<value_type>)
        return std::forward<value_type>(*wrapped.handle().promise().result);
    else 
        return;
}

}

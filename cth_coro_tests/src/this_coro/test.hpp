#pragma once

#include "../test.hpp"
#include "cth/coro/executor.hpp"
#include "cth/coro/sync.hpp"
#include "cth/coro/tasks/sync_task.hpp"

#define THIS_CORO_EX_TEST(ex, suite, name) CORO_EX_TEST(_this_coro##ex, suite, name)

#define THIS_CORO_TEST(suite, name) THIS_CORO_EX_TEST(, suite, name)

namespace cth::co {


template<class Task>
awaited_t<Task> sync_wait(executor& exec, Task task) {
    return co::sync(exec.spawn(std::move(task)));
}

}

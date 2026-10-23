#include "cth/coro/utility/concepts.hpp"

#include "cth/coro/tasks/dev/capture_task.hpp"
#include "cth/coro/tasks/dev/spawn_task.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/tasks/sync_task.hpp"


namespace cth::co {
// the result of steal() returns to the payload context of the steal itself
static_assert(captured_awaitable<capture_task<int>>);
static_assert(captured_awaitable<capture_task<void>>);
static_assert(!foreign_awaitable<capture_task<int>>);
static_assert(!this_coro_awaitable<capture_task<int>>);

// the result of spawn() takes the scheduler of the caller from its payload
static_assert(this_coro_awaitable<spawn_task<int>>);
static_assert(this_coro_awaitable<spawn_task<void>>);
static_assert(!captured_awaitable<spawn_task<int>>);
static_assert(!foreign_awaitable<spawn_task<int>>);

static_assert(this_coro_awaitable<executor_task<int>>);
static_assert(!captured_awaitable<executor_task<int>>);
static_assert(!foreign_awaitable<executor_task<int>>);

static_assert(this_coro_awaitable<sync_executor_task<int>>);
static_assert(!captured_awaitable<sync_executor_task<int>>);
static_assert(!foreign_awaitable<sync_executor_task<int>>);

static_assert(foreign_awaitable<sync_task<int>>);
static_assert(!captured_awaitable<sync_task<int>>);
static_assert(!this_coro_awaitable<sync_task<int>>);
}

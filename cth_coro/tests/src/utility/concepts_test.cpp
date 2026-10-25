#include "cth/coro/utility/concepts.hpp"

#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/tasks/dev/capture_task.hpp"
#include "cth/coro/tasks/dev/spawn_task.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/tasks/sync_task.hpp"
#include "cth/coro/this_coro.hpp"


namespace cth::co {
// the capture of a foreign awaitable continues the caller on its scheduler
static_assert(this_coro_awaitable<capture_task<int>>);
static_assert(this_coro_awaitable<capture_task<void>>);
static_assert(!foreign_awaitable<capture_task<int>>);

// the result of spawn() continues the caller on its scheduler
static_assert(this_coro_awaitable<spawn_task<int>>);
static_assert(this_coro_awaitable<spawn_task<void>>);
static_assert(!foreign_awaitable<spawn_task<int>>);

static_assert(this_coro_awaitable<executor_task<int>>);
static_assert(this_coro_awaitable<executor_task<int>&>);
static_assert(!foreign_awaitable<executor_task<int>>);

static_assert(this_coro_awaitable<schedule_awaiter>);
static_assert(this_coro_awaitable<schedule_awaiter&>);
static_assert(!foreign_awaitable<schedule_awaiter>);

static_assert(foreign_awaitable<sync_task<int>>);
static_assert(!this_coro_awaitable<sync_task<int>>);

// tags are neither
static_assert(!this_coro_awaitable<this_coro::context_tag>);
static_assert(!foreign_awaitable<this_coro::context_tag>);
static_assert(!foreign_awaitable<this_coro::context_tag const&>);
}

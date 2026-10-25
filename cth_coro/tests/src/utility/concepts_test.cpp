#include "cth/coro/utility/concepts.hpp"

#include "cth/coro/awaiters/schedule_awaiter.hpp"
#include "cth/coro/tasks/dev/capture_task.hpp"
#include "cth/coro/tasks/dev/spawn_task.hpp"
#include "cth/coro/tasks/executor_task.hpp"
#include "cth/coro/tasks/sync_task.hpp"
#include "cth/coro/this_coro.hpp"


namespace cth::co {
// the capture of a foreign awaitable continues the caller on its context
static_assert(this_coro_awaitable<capture_task<int>>);
static_assert(this_coro_awaitable<capture_task<void>>);
static_assert(!foreign_awaitable<capture_task<int>>);

// the result of spawn() continues the caller on its context
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

namespace {
    struct caller_promise {};

    struct nothrow_attach_promise {
        template<class Caller>
        void attach(Caller const&) noexcept {}
    };

    struct throwing_attach_promise {
        template<class Caller>
        void attach(Caller const&) {}
    };
}

// a promise attaches to its caller, nothrow if the attach is noexcept
static_assert(dev::nothrow_attachable<nothrow_attach_promise, caller_promise>);
static_assert(!dev::nothrow_attachable<throwing_attach_promise, caller_promise>);
static_assert(!dev::nothrow_attachable<caller_promise, caller_promise>);

// tags are neither
static_assert(!this_coro_awaitable<this_coro::context_tag>);
static_assert(!foreign_awaitable<this_coro::context_tag>);
static_assert(!foreign_awaitable<this_coro::context_tag const&>);
}

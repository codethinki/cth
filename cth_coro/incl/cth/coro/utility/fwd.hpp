#pragma once

#include <cth/chrono.hpp>


namespace cth::co {
class scheduler;
class executor;
}


namespace cth::co {
template<class T> class executor_task;
template<class T> class capture_task;
template<class T> class scheduled_task;
}


namespace cth::co {
struct schedule_awaiter;
struct wait_awaiter;

struct this_coro_promise_awaiter_base;
struct this_coro_promise_base;
struct capture_awaiter_base;
struct sync_promise_base;

}


namespace cth::co::this_coro {
struct payload;

struct [[nodiscard]] tag_base {};
struct executor_tag;
struct scheduler_tag;

struct wait_tag;

}

namespace cth::except {
class coro_exception;
}

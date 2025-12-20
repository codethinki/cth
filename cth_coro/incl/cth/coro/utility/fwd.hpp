#pragma once

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

struct this_coro_awaiter_base;
struct this_coro_promise_base;
struct capture_awaiter_base;
struct sync_promise_base;

}

namespace cth::co::this_coro {
struct executor_tag;
struct scheduler_tag;
}

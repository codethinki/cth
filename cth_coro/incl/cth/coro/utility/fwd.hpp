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

struct executor_awaiter_base;
struct executor_promise_base;
struct capture_awaiter_base;
struct sync_promise_base;

}

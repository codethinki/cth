#pragma once
#include "independent_concepts.hpp"

namespace cth::co {
class scheduler;
class executor;
}

namespace cth::co::this_coro {
struct scheduler_payload_base;
using default_payload = scheduler_payload_base;


/**
 * new this_coro tags must inherit from this
 */
struct [[nodiscard]] tag_base {};
struct executor_tag;
struct scheduler_tag;

struct wait_tag;

}

namespace cth::co {
template<class T, payload Pyld = this_coro::default_payload>
class executor_task;

template<class T, payload Pyld = this_coro::default_payload>
class scheduled_task;

template<class T, payload Pyld = this_coro::default_payload>
class sync_executor_task;


template<class T>
class capture_task;
}



namespace cth::except {
class coro_exception;
}

namespace cth::co {
struct schedule_awaiter;
struct wait_awaiter;

template<payload Pyld = this_coro::default_payload>
struct this_coro_promise_awaiter_base;
template<payload Pyld = this_coro::default_payload>
struct this_coro_promise_base;

struct capture_awaiter_base;
struct sync_promise_base;

}

#pragma once
#include "basic_promise_awaiter.hpp"

#include "cth/coro/utility/fwd.hpp"

namespace cth::co::dev {

template<class Promise>
struct capture_promise_awaiter
    : basic_promise_awaiter<Promise>
    , capture_awaiter_base {
    using dev::basic_promise_awaiter<Promise>::basic_promise_awaiter;
};
}

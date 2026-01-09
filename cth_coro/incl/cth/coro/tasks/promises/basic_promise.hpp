#pragma once
#include "raw_promise.hpp"

#include "cth/coro/tasks/awaiters/dev/basic_final_promise_awaiter.hpp"
#include <coroutine>

namespace cth::co {
template<class T, auto InitS = std::suspend_always{}, auto FinalS = dev::basic_final_promise_awaiter{}>
struct basic_promise : raw_promise<T> {
    using initial_suspend_type = decltype(InitS);
    using final_suspend_type = decltype(FinalS);

    constexpr auto initial_suspend() noexcept { return InitS; }
    constexpr auto final_suspend() noexcept { return FinalS; }
};
}

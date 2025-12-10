#pragma once
#include "raw_promise.hpp"

#include "cth/coro/awaiters/dev/basic_final_awaiter.hpp"
#include <coroutine>

namespace cth::co {
template<class T, auto InitS = std::suspend_always{}, auto FinalS = dev::basic_final_awaiter{}>
struct basic_promise : raw_promise<T> {
    using initial_suspend_type = decltype(InitS);
    using final_suspend_type = decltype(FinalS);

    auto initial_suspend() noexcept { return InitS; }
    auto final_suspend() noexcept { return FinalS; }
};
}

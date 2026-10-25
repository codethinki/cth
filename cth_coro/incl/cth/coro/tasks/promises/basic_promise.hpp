#pragma once
#include "raw_promise.hpp"

#include <coroutine>

namespace cth::co {
/**
 * promise with a fixed initial and final awaiter
 * @tparam T result type
 * @tparam InitS initial awaiter
 * @tparam FinalS final awaiter
 */
template<class T, auto InitS, auto FinalS>
struct basic_promise : raw_promise<T> {
    using initial_suspend_type = decltype(InitS);
    using final_suspend_type = decltype(FinalS);

    constexpr auto initial_suspend() noexcept { return InitS; }
    constexpr auto final_suspend() noexcept { return FinalS; }
};
}

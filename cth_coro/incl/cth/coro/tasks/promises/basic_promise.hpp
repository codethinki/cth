#pragma once
#include "raw_promise.hpp"
#include "suspend_promise_base.hpp"


namespace cth::co {
/**
 * promise with the result, the exception and a fixed initial and final awaiter
 * @tparam T result type
 * @tparam InitS initial awaiter
 * @tparam FinalS final awaiter
 */
template<class T, auto InitS, auto FinalS>
struct basic_promise : raw_promise<T>, suspend_promise_base<InitS, FinalS> {};
}

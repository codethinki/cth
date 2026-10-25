#pragma once
#include "native_handle_awaiter_base.hpp"

#include "cth/os/fence.hpp"

namespace cth::co {
/**
 * base of awaiters that wait on an @ref os::fence
 * @details derived awaiters add await_resume()
 */
struct osfence_awaiter_base : native_handle_awaiter_base {
    explicit osfence_awaiter_base(os::fence const& fence) : native_handle_awaiter_base{fence.native_handle()} {}
};
}

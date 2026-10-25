#pragma once
#include "native_handle_awaiter_base.hpp"

#include "cth/os/fence.hpp"

namespace cth::co {
struct osfence_awaiter_base : native_handle_awaiter_base {
    explicit osfence_awaiter_base(os::fence const& fence) : native_handle_awaiter_base{fence.native_handle()} {}
};
}

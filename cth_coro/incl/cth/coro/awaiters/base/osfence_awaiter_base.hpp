#pragma once
#include "native_handle_awaiter_base.hpp"

#include "cth/os/fence.hpp"

namespace cth::co {
template<this_coro::scheduler_payload Pyld = this_coro::default_payload>
struct osfence_awaiter_base : native_handle_awaiter_base<Pyld> {
    explicit osfence_awaiter_base(os::fence const& fence) : native_handle_awaiter_base<Pyld>{
        fence.native_handle()} {}
};
}

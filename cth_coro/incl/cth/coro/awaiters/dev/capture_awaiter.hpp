#pragma once
#include "basic_awaiter.hpp"
#include "capture_awaiter_base.hpp"

namespace cth::co::dev {

template<class Promise>
struct capture_awaiter : basic_awaiter<Promise>, capture_awaiter_base {
    using dev::basic_awaiter<Promise>::basic_awaiter;
};
}

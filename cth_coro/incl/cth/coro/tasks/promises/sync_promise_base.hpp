#pragma once
#include "basic_promise.hpp"

#include "cth/coro/awaiters/final_sync_awaiter.hpp"
#include "cth/coro/atomic_fence.hpp"

namespace cth::co {
struct sync_promise_base {
    void wait() const noexcept { fence.wait(); }


    [[nodiscard]] bool signaled() const noexcept { return fence.signaled(); }

    void signal() noexcept { fence.signal(); }

    atomic_fence fence{};
};
}

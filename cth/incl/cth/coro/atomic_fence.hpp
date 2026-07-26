#pragma once
#include "cth/coro/utility.hpp"

#include <atomic>
#include <memory>

namespace cth::co {

class atomic_fence {

public:
    atomic_fence() noexcept : _state{false} {}
    explicit atomic_fence(signaled_t) noexcept : _state{true} {}

    ~atomic_fence() { signal(); }

    void signal() noexcept {
        if(!_state.exchange(true, std::memory_order_release))
            _state.notify_all();
    }
    void force_signal() noexcept {
        _state.store(true, std::memory_order_release);
        _state.notify_all();
    }

    void reset() noexcept { _state.store(false, std::memory_order_release); }

    void wait() const noexcept { _state.wait(false, std::memory_order_acquire); }

private:
    std::atomic<bool> _state;

public:
    bool signaled() const noexcept { return _state.load(std::memory_order_acquire); }

    atomic_fence(atomic_fence const&) = delete;
    atomic_fence& operator=(atomic_fence const&) = delete;
    atomic_fence(atomic_fence&&) = delete;
    atomic_fence& operator=(atomic_fence&&) = delete;
};
}

namespace cth::co {

class atomic_heap_fence {

public:
    atomic_heap_fence() : _fence(std::make_unique<atomic_fence>()) {}
    explicit atomic_heap_fence(signaled_t) : _fence(std::make_unique<atomic_fence>(co::signaled)) {}

    ~atomic_heap_fence() = default;

    void signal() {
        if(_fence)
            _fence->signal();
    }

    void force_signal() {
        if(_fence)
            _fence->force_signal();
    }

    void reset() {
        if(_fence)
            _fence->reset();
    }

    void wait() const {
        if(_fence)
            _fence->wait();
    }

private:
    std::unique_ptr<atomic_fence> _fence;

public:
    bool signaled() const { return _fence && _fence->signaled(); }

    atomic_heap_fence(atomic_heap_fence&&) noexcept = default;
    atomic_heap_fence& operator=(atomic_heap_fence&&) noexcept = default;

    atomic_heap_fence(atomic_heap_fence const&) = delete;
    atomic_heap_fence& operator=(atomic_heap_fence const&) = delete;
};

}

#pragma once

#include <atomic>
#include <memory>

namespace cth::co {

struct signaled_t {};
inline constexpr signaled_t signaled{};


class fence {

public:
    fence() noexcept : _state{false} {}
    explicit fence(signaled_t) noexcept : _state{true} {}

    ~fence() { signal(); }

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


    fence(fence const&) = delete;
    fence& operator=(fence const&) = delete;
    fence(fence&&) = delete;
    fence& operator=(fence&&) = delete;

};
}

namespace cth::co {

class heap_fence {

public:
    heap_fence() : _fence(std::make_unique<fence>()) {}
    explicit heap_fence(signaled_t) : _fence(std::make_unique<fence>(co::signaled)) {}

    ~heap_fence() = default;

    void signal() { if(_fence) _fence->signal(); }

    void force_signal() { if(_fence) _fence->force_signal(); }


    void reset() { if(_fence) _fence->reset(); }

    void wait() const { if(_fence) _fence->wait(); }

private:
    std::unique_ptr<fence> _fence;

public:
    bool signaled() const { return _fence && _fence->signaled(); }

    heap_fence(heap_fence&&) noexcept = default;
    heap_fence& operator=(heap_fence&&) noexcept = default;

    heap_fence(heap_fence const&) = delete;
    heap_fence& operator=(heap_fence const&) = delete;

};

}

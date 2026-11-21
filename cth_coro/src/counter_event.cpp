#include "cth/coro/counter_event.hpp"

#include <cth/io/log.hpp>


namespace cth::co {
counter_event::~counter_event() {
    CTH_CRITICAL(_head.load(std::memory_order_acquire) != nullptr, "the event must have no pending wait") {}
}

void counter_event::set(uint64_t value) noexcept {
    CTH_CRITICAL(value <= this->value(), "the value must rise, got {} after {}", value, this->value()) {}

    // release: a wait that sees the value sees what happened before the set()
    _value.store(value, std::memory_order_release);
    drain();
}

void counter_event::push(node& wait) const noexcept {
    // acq_rel: the drain that takes the node sees its fields
    // a push after the drain of a set() sees the value of the set()
    auto* head = _head.load(std::memory_order_relaxed);
    do wait.next = head;
    while(!_head.compare_exchange_weak(head, &wait, std::memory_order_acq_rel, std::memory_order_relaxed));
}

void counter_event::drain() const noexcept {
    while(true) {
        auto* waits = _head.exchange(nullptr, std::memory_order_acq_rel);
        if(waits == nullptr) return;

        auto const value = this->value();
        while(waits != nullptr) {
            auto& wait = *waits;
            // the resume can end the wait at once: nothing touches the node after the post
            waits = wait.next;
            if(wait.value > value) {
                push(wait);
                continue;
            }

            auto const caller = wait.caller;
            auto const& context = *wait.context;
            context.post([caller] { caller.resume(); });
        }

        // a set() during the pass found no node: it did not see the pushed back waits
        if(this->value() == value) return;
    }
}

counter_event::counter_event(counter_event&& other) noexcept : _value{other.value()} {
    CTH_CRITICAL(
        other._head.load(std::memory_order_acquire) != nullptr,
        "the moved event must have no pending wait"
    ) {}
}

counter_event& counter_event::operator=(counter_event&& other) noexcept {
    CTH_CRITICAL(
        _head.load(std::memory_order_acquire) != nullptr
        || other._head.load(std::memory_order_acquire) != nullptr,
        "both events must have no pending wait"
    ) {}

    _value.store(other.value(), std::memory_order_release);
    return *this;
}
}

#pragma once
#include "cth/coro/utility.hpp"

#include <functional>


namespace cth::co {
/**
 * switches global state on/off based on acquire/release calls
 * @details this is designed to be thread safe
 * @note by default has RAII switch off on destruction
 */
class global_switch {

public:
    using void_func = std::move_only_function<void()>;

    global_switch(void_func on, void_func off) : _on{std::move(on)}, _off{std::move(off)} {}

    global_switch(signaled_t, void_func on, void_func off) : global_switch{std::move(on), std::move(off)} {
        acquire();
    }

    /**
     * calls @ref force_off()
     */
    ~global_switch() {
        force_off();
    }

    /**
     * acquires the switch, only calls on if count was 0
     * @return true if on was called
     */
    bool acquire() {
        if(_count.fetch_add(1) == 0) {
            _on();
            return true;
        }
        return false;
    }

    /**
     * releases the switch, only calls off if acquire count is 0
     * @attention calling release without a previous acquire is UB
     * @return true if off was called
     */
    bool release() {
        auto const count = _count.fetch_sub(1);

        if(count == 1) {
            _off();
            return true;
        }
        return false;
    }

    /**
     * forces the switch off if not already
     * @attention all previous acquires are discarded, calling release for a previous acquire is UB.
     */
    void force_off() {
        if(_count.load() == 0) return;

        _off();
        _count = 0;
    }

private:
    std::atomic<size_t> _count{};

    void_func _on;
    void_func _off;

public:
    /**
     * gets the amount of outstanding acquires
     */
    [[nodiscard]] size_t acquire_count() const {
        return _count.load();
    }

    [[nodiscard]] bool acquired() const {
        return acquire_count() > 0;
    }
};
}

#pragma once

#include "cth/io/log.hpp"
#include "cth/types.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cth::dt {

enum class cleanup_t : u64 {};

namespace dev {
    using raw_cleanup_t = std::underlying_type_t<cleanup_t>;

    constexpr uz CLEANUP_COUNTER_BITS = sizeof(cleanup_t) / 4 * 8;
    constexpr uz CLEANUP_ALLOC_SIZE_BITS = sizeof(cleanup_t) * 8 - CLEANUP_COUNTER_BITS;
}

struct ring_allocation {
    uz begin;
    cleanup_t cleanup;
};


/**
 * ring allocator with contiguous allocation guarantee
 * @details
 * - operates under FIFO / cumulative free assumption, i.e. freeing allocation B after A assumes A can be overwritten
 * - fully thread safe
 */
class ring_alloc {
    using pos_t = u64;
    using counter_t = u16;

public:
    /**
     * creates the ring allocator for a given buffer size
     * @param size in bytes
     */
    constexpr ring_alloc(uz size) : _size{size} {}

    /**
     * allocates
     * @param size to allocate (`< max_alloc()`)
     * @return alloc info or empty if not enough space
     */
    [[nodiscard]] std::optional<ring_allocation> alloc(uz size) {
        auto head = _head.load(std::memory_order::relaxed);

        uz begin;
        uz totalSize;

        // cas
        while(true) {
            auto const tail = _tail.load(std::memory_order::acquire);

            auto const actualHead = head % _size;

            // ring wrap
            bool const wrap = (actualHead + size) > _size;

            begin = wrap ? 0 : actualHead;

            totalSize = size;

            // add wrap padding
            if(wrap)
                totalSize += _size - actualHead;

            // not enough space
            if(head + totalSize - tail > _size)
                return std::nullopt;

            // try commit
            if(_head.compare_exchange_weak(
                head,
                head + totalSize,
                std::memory_order::release,
                std::memory_order::relaxed
            ))
                break; // Success, we secured the space!
        }

        return ring_allocation{begin, create_token(head, totalSize)};
    }

    /**
     * frees the allocation.
     * @param token to free
     * @details cumulative free => freeing B before A assumes A is free as well
     */
    void free(cleanup_t token) {
        auto tail = _tail.load(std::memory_order::acquire);
        auto const pos = end(token);

        while(
            tail < pos
            && !_tail.compare_exchange_weak(
                tail,
                pos,
                std::memory_order::release,
                std::memory_order::relaxed
            )) {}
    }



    [[nodiscard]] constexpr uz size() const { return _size; }
    /**
     * alias for size
     */
    [[nodiscard]] constexpr uz capacity() const { return size(); }

    /**
     * allocated bytes
     * @note may not be used to determine the max allocation, use `max_alloc()` instead
     */
    [[nodiscard]] uz allocated() const {
        return _head.load(std::memory_order::relaxed) - _tail.load(std::memory_order::relaxed);
    }

    /**
     * gets the current max allocation size
     * @return size in bytes
     * @note in highly threaded environments this may not be accurate
     */
    [[nodiscard]] uz max_alloc() const {
        auto const head = _head.load(std::memory_order::relaxed);
        auto const tail = _tail.load(std::memory_order::relaxed);

        if(head - tail >= capacity())
            return 0;

        auto const actualHead = head % _size;
        auto const actualTail = tail % _size;

        // [..., h <..free..> t ...]
        if(actualHead < actualTail)
            return actualTail - actualHead;

        // [..> t ... h <..free]
        auto const naiveMax = _size - actualHead;
        auto const wrapMax = actualTail;

        return std::max(naiveMax, wrapMax);
    }

private:
    [[nodiscard]] static constexpr cleanup_t create_token(uz head, uz actual_alloc_size) {
        return static_cast<cleanup_t>(head + actual_alloc_size);
    }

    [[nodiscard]] static constexpr uz end(cleanup_t token) { return static_cast<uz>(token); }

    uz _size;
    std::atomic<pos_t> _head{};
    std::atomic<pos_t> _tail{};
};

}

#pragma once

#include "cth/io/log.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cth::dt {

enum class cleanup_t : uint64_t {};

namespace dev {
    using raw_cleanup_t = std::underlying_type_t<cleanup_t>;

    constexpr size_t CLEANUP_COUNTER_BITS = sizeof(cleanup_t) / 4 * 8;
    constexpr size_t CLEANUP_ALLOC_SIZE_BITS = sizeof(cleanup_t) * 8 - CLEANUP_COUNTER_BITS;
}

struct ring_allocation {
    size_t begin;
    cleanup_t cleanup;
};


/**
 * ring allocator with contiguous allocation guarantee
 * @details
 * - operates under FIFO / cumulative free assumption, i.e. freeing allocation B after A assumes A can be overwritten
 * - fully thread safe
 */
class ring_alloc {
    using pos_t = uint64_t;
    using counter_t = uint16_t;

public:
    /**
     * creates the ring allocator for a given buffer size
     * @param size in bytes
     */
    constexpr ring_alloc(size_t size) : _size{size} {}

    /**
     * allocates
     * @param size to allocate (< @ref max_alloc())
     * @return alloc info or empty if not enough space
     */
    [[nodiscard]] std::optional<ring_allocation> alloc(size_t size) {
        auto head = _head.load(std::memory_order::relaxed);

        size_t begin;
        size_t totalSize;

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



    [[nodiscard]] constexpr size_t size() const { return _size; }
    /**
     * alias for size
     */
    [[nodiscard]] constexpr size_t capacity() const { return size(); }

    /**
     * allocated bytes
     * @note may not be used to determine the max allocation, use @ref max_alloc() instead
     */
    [[nodiscard]] size_t allocated() const {
        return _head.load(std::memory_order::relaxed) - _tail.load(std::memory_order::relaxed);
    }

    /**
     * gets the current max allocation size
     * @return size in bytes
     * @note in highly threaded environments this may not be accurate
     */
    [[nodiscard]] size_t max_alloc() const {
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
    [[nodiscard]] static constexpr cleanup_t create_token(size_t head, size_t actual_alloc_size) {
        return static_cast<cleanup_t>(head + actual_alloc_size);
    }

    [[nodiscard]] static constexpr size_t end(cleanup_t token) { return static_cast<size_t>(token); }

    size_t _size;
    std::atomic<pos_t> _head{};
    std::atomic<pos_t> _tail{};
};

}

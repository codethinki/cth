module;

#include <array>
#include <bit>
#include <cstdint>
#include <vector>

export module cth.data.miniram:decl;

export namespace cth::dt {

template<class T>
concept uint = std::unsigned_integral<T>;

template<uint T>
consteval auto invalid() { return T{} - 1; }

namespace dev {
    // Internal node structure, made public for development/debugging purposes.
    template<uint SizeType, uint IndexType>
    struct basic_mini_node {
        using index_t = IndexType;
        using size_t = SizeType;
        static constexpr index_t UNUSED = invalid<index_t>();

        size_t dataOffset = 0;
        size_t dataSize = 0;

        index_t binListPrev = UNUSED;
        index_t binListNext = UNUSED;
        index_t neighborPrev = UNUSED;
        index_t neighborNext = UNUSED;
        bool used = false;
    };
}

template<uint SizeType, uint IndexType>
struct basic_mini_alloc {
    SizeType offset;
    IndexType id;
};

template<uint T>
struct basic_mini_region {
    size_t count;
    T size;
};

template<uint T>
struct basic_mini_memmove {
    T srcOffset;
    T dstOffset;
    T size;
};

template<uint SizeType, uint IndexType>
struct basic_mini_defrag {
    std::vector<basic_mini_alloc<SizeType, IndexType>> updatedAllocs;
    std::vector<basic_mini_memmove<SizeType>> moves;
};


template<uint SizeType, uint IndexType>
class basic_miniram {
public:
    using size_type = SizeType;
    using index_type = IndexType;
    using alloc_type = basic_mini_alloc<SizeType, IndexType>;
    using regions_type = basic_mini_region<SizeType>;
    using memmove_type = basic_mini_memmove<SizeType>;
    using defrag_type = basic_mini_defrag<SizeType, IndexType>;
    using node_type = dev::basic_mini_node<SizeType, IndexType>;

private:
    // Bin configuration - calculated from BINS_PER_LEAF
    static constexpr size_t BINS_PER_LEAF = 8;
    static constexpr size_t MANTISSA_BITS = std::countr_zero(BINS_PER_LEAF); // log2(8) = 3
    static constexpr size_type MANTISSA_VALUE = size_type{1} << MANTISSA_BITS;
    static constexpr size_type MANTISSA_MASK = MANTISSA_VALUE - 1;

    static constexpr size_t NUM_TOP_BINS = sizeof(SizeType) * 8;
    static constexpr size_t TOP_BINS_INDEX_SHIFT = MANTISSA_BITS;
    static constexpr size_type LEAF_BINS_INDEX_MASK = BINS_PER_LEAF - 1;
    static constexpr size_t NUM_LEAF_BINS = NUM_TOP_BINS * BINS_PER_LEAF;

    static constexpr size_t ALLOC_GROW_FACTOR = 2;

    // Bin mask types
    using top_bin_mask_t = std::conditional_t<sizeof(SizeType) <= 4, uint32_t, uint64_t>;
    using leaf_bin_mask_t = uint8_t; // Always 8 bins per leaf

public:
    static constexpr index_type INVALID_INDEX = invalid<IndexType>();
    static constexpr size_type NO_SPACE = invalid<SizeType>();

    /**
     * constructs
     * @param capacity of ram (in elements)
     * @param initial_alloc_capacity (in allocations), >= 1
     */
    constexpr explicit basic_miniram(size_type capacity, size_t initial_alloc_capacity = 128 * 1024);
    constexpr ~basic_miniram() = default;

    /**
     * clears the ram to no allocations
     */
    constexpr void clear();

    /**
     * allocates a block, may fail
     * @param size (in elements)
     * @return allocation (offset, id) or (NO_SPACE, NO_SPACE)
     */
    [[nodiscard]] constexpr alloc_type allocate(size_type size);

    /**
     * frees an allocation
     * @param allocation to free
     */
    constexpr void free(alloc_type allocation);

    /**
     * queries an allocation's size
     * @param allocation to check
     * @return size in elements
     */
    [[nodiscard]] constexpr size_type size_of(alloc_type allocation) const;

    /**
     * array of free regions in ram (count, size)
     */
    [[nodiscard]] constexpr std::array<regions_type, NUM_LEAF_BINS> free_regions() const;

    /**
     * max allocatable block size (in elements)
     */
    [[nodiscard]] constexpr size_type max_alloc() const;

    /**
     * defragments the ram (left compaction).
     * @return defragmentation report containing allocation changes and move operations
     */
    [[nodiscard]] constexpr defrag_type defragment() { return defragment(capacity()); }

    /**
     * resizes and defragments the ram
     * @param new_capacity to resize to
     * @return defragmentation report
     */
    [[nodiscard]] constexpr defrag_type resize(size_type new_capacity) { return defragment(new_capacity); }

    constexpr void reserve_allocations(size_t alloc_capacity) { reserve_nodes(alloc_capacity + 1); }

    /**
     * splits moves into disjunct copies (no overlapping regions)
     * @param moves to convert
     */
    constexpr static std::vector<memmove_type> to_disjunct_copies(std::vector<memmove_type> const& moves);

private:
    // Float conversion functions - integrated into class
    [[nodiscard]] constexpr static size_type highest_bit(size_type number);
    [[nodiscard]] constexpr static size_type ceil_to_float(size_type size);
    [[nodiscard]] constexpr static size_type floor_to_float(size_type size);
    [[nodiscard]] constexpr static size_type to_uint(size_type float_value);

    constexpr void reserve_allocations() { reserve_allocations(alloc_capacity() * ALLOC_GROW_FACTOR); }
    constexpr void reserve_nodes(size_t node_capacity);

    constexpr size_type compactUsedNodes(std::vector<index_type> const& used_nodes, defrag_type& report);
    constexpr void reconstructFreeSpace(size_type compacted_offset, index_type last_used_node);

    constexpr void sliceTopBinNode(size_type slice_size, size_t bin_id, size_t top_bin_id, size_t leaf_bin_id);
    constexpr static size_t findLowestBitAfter(top_bin_mask_t bit_mask, size_t start_id);

    constexpr index_type insertNode(size_type size, size_type data_offset);
    constexpr void removeNode(index_type node_index);

    [[nodiscard]] constexpr index_type newNode(size_type size);
    constexpr void freeNode(index_type node_index);

    constexpr defrag_type defragment(size_type new_size);
    constexpr void defragmentReset();

    [[nodiscard]] constexpr size_t nodes() const { return _nodes.size(); }
    [[nodiscard]] constexpr index_type popFreeNode();
    [[nodiscard]] constexpr index_type& pushFreeNode();

    size_type _capacity;
    size_t _maxAllocs;
    size_type _freeStorage{};

    top_bin_mask_t _usedBinsTop{};
    std::array<leaf_bin_mask_t, NUM_TOP_BINS> _usedBins{};
    std::array<index_type, NUM_LEAF_BINS> _binIndices{};

    std::vector<node_type> _nodes{};
    std::vector<index_type> _freeNodes{};
    size_t _freeStackPtr{};

public:
    /**
     * amount of elements in ram
     */
    [[nodiscard]] constexpr size_type capacity() const { return _capacity; }
    /**
     * gets the current max number of allocations.
     * @details not a hard limit
     */
    [[nodiscard]] constexpr size_t alloc_capacity() const { return nodes() - 1; }
    /**
     * Gets the remaining reserveless allocations
     * @details not a hard limit.
     */
    [[nodiscard]] constexpr size_t remaining_allocs() const { return nodes() - _freeStackPtr; }
    /**
     * amount of unallocated elements left in ram
     */
    [[nodiscard]] constexpr size_type remaining() const { return _freeStorage; }
    /**
     * amount of allocated elements in ram
     */
    [[nodiscard]] constexpr size_type allocated() const { return capacity() - remaining(); }

    /**
     * true if no more elements can be allocated
     */
    [[nodiscard]] constexpr bool exhausted() const { return remaining() == 0; }
    /**
     * true if nothing is allocated
     */
    [[nodiscard]] constexpr bool empty() const { return remaining() == capacity(); }
    /**
     * checks if a given size can be allocated
     */
    [[nodiscard]] constexpr bool allocatable(size_type size) const { return size <= max_alloc(); }

    /**
     * rates the fragmentation [0, 1] (good to bad)
     */
    [[nodiscard]] constexpr float fragmentation() const {
        if(remaining() == 0) return 0.0f;
        return 1.0f - static_cast<float>(max_alloc()) / static_cast<float>(remaining());
    }

    constexpr basic_miniram(basic_miniram const& other) = default;
    constexpr basic_miniram(basic_miniram&& other) noexcept = default;
    constexpr basic_miniram& operator=(basic_miniram const& other) = default;
    constexpr basic_miniram& operator=(basic_miniram&& other) noexcept = default;
};

using miniram32 = basic_miniram<uint32_t, uint32_t>;
using miniram64 = basic_miniram<uint64_t, uint32_t>;
using miniram = basic_miniram<size_t, uint32_t>;

using mini_node32 = dev::basic_mini_node<uint32_t, uint32_t>;
using mini_node64 = dev::basic_mini_node<uint64_t, uint32_t>;
using mini_node = dev::basic_mini_node<size_t, uint32_t>;

using mini_alloc32 = basic_mini_alloc<uint32_t, uint32_t>;
using mini_alloc64 = basic_mini_alloc<uint64_t, uint32_t>;
using mini_alloc = basic_mini_alloc<size_t, uint32_t>;

using mini_region32 = basic_mini_region<uint32_t>;
using mini_region64 = basic_mini_region<uint64_t>;
using mini_region = basic_mini_region<size_t>;

using mini_memmove32 = basic_mini_memmove<uint32_t>;
using mini_memmove64 = basic_mini_memmove<uint64_t>;
using mini_memmove = basic_mini_memmove<size_t>;

using mini_defrag32 = basic_mini_defrag<uint32_t, uint32_t>;
using mini_defrag64 = basic_mini_defrag<uint64_t, uint32_t>;
using mini_defrag = basic_mini_defrag<size_t, uint32_t>;

}

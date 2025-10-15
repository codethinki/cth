#include "cth/memory/miniram.hpp"

#include "cth/test.hpp"

#include <array>
#include <cstdint>
#include <random>
#include <vector>


// Custom test macro as requested
#define MEM_TEST(suite, name) CTH_EX_TEST(_mem, suite, name)

#define MEM_EX_TEST(category, suite, name) CTH_EX_TEST(_mem##category, suite, name)


namespace cth::mem {
using miniram_t = miniram<>;
using mini_alloc_t = mini_alloc<>;

MEM_TEST(miniram, BasicOperations) {
    miniram_t allocator(1024 * 1024 * 256);
    auto a = allocator.allocate(1337);
    EXPECT_EQ(a.offset, 0);
    allocator.free(a);
}

MEM_TEST(miniram, SimpleSequence) {
    miniram_t allocator(1024 * 1024 * 256);
    auto a = allocator.allocate(0);
    EXPECT_EQ(a.offset, 0);

    auto b = allocator.allocate(1);
    EXPECT_EQ(b.offset, 0);

    auto c = allocator.allocate(123);
    EXPECT_EQ(c.offset, 1);

    auto d = allocator.allocate(1234);
    EXPECT_EQ(d.offset, 124);

    allocator.free(a);
    allocator.free(b);
    allocator.free(c);
    allocator.free(d);

    auto validateAll = allocator.allocate(1024 * 1024 * 256);
    EXPECT_EQ(validateAll.offset, 0);
    allocator.free(validateAll);
}

MEM_TEST(miniram, TrivialMerge) {
    miniram_t allocator(1024 * 1024 * 256);
    auto a = allocator.allocate(1337);
    EXPECT_EQ(a.offset, 0);
    allocator.free(a);

    auto b = allocator.allocate(1337);
    EXPECT_EQ(b.offset, 0);
    allocator.free(b);

    auto validateAll = allocator.allocate(1024 * 1024 * 256);
    EXPECT_EQ(validateAll.offset, 0);
}

MEM_TEST(miniram, TrivialReuse) {
    miniram_t allocator(1024 * 1024 * 256);
    auto a = allocator.allocate(1024);
    EXPECT_EQ(a.offset, 0);

    auto b = allocator.allocate(3456);
    EXPECT_EQ(b.offset, 1024);

    allocator.free(a);

    auto c = allocator.allocate(1024);
    EXPECT_EQ(c.offset, 0);

    allocator.free(c);
    allocator.free(b);

    auto validateAll = allocator.allocate(1024 * 1024 * 256);
    EXPECT_EQ(validateAll.offset, 0);
}

MEM_TEST(miniram, ComplexReuseAndFragmentation) {
    miniram_t allocator(1024 * 1024 * 256);
    auto a = allocator.allocate(1024);
    EXPECT_EQ(a.offset, 0);

    auto b = allocator.allocate(3456);
    EXPECT_EQ(b.offset, 1024);

    allocator.free(a);

    auto c = allocator.allocate(2345);
    EXPECT_EQ(c.offset, 1024 + 3456);

    auto d = allocator.allocate(456);
    EXPECT_EQ(d.offset, 0);

    auto e = allocator.allocate(512);
    EXPECT_EQ(e.offset, 456);

    EXPECT_EQ(allocator.remaining(), 1024 * 1024 * 256 - 3456 - 2345 - 456 - 512);
    EXPECT_NE(allocator.max_alloc(), allocator.remaining());

    allocator.free(c);
    allocator.free(d);
    allocator.free(b);
    allocator.free(e);

    auto validateAll = allocator.allocate(1024 * 1024 * 256);
    EXPECT_EQ(validateAll.offset, 0);
}

MEM_TEST(miniram, ZeroFragmentationScenario) {
    miniram_t ram(1024 * 1024 * 256);
    constexpr uint32_t numAllocs = 256;
    constexpr uint32_t allocSize = 1024 * 1024;

    std::vector<mini_alloc_t> allocations(numAllocs);

    for(uint32_t i = 0; i < numAllocs; i++) {
        allocations[i] = ram.allocate(allocSize);
        EXPECT_EQ(allocations[i].offset, i * allocSize);
    }

    EXPECT_EQ(ram.remaining(), 0);
    EXPECT_EQ(ram.max_alloc(), 0);

    ram.free(allocations[243]);
    ram.free(allocations[5]);
    ram.free(allocations[123]);
    ram.free(allocations[95]);

    ram.free(allocations[151]);
    ram.free(allocations[152]);
    ram.free(allocations[153]);
    ram.free(allocations[154]);

    allocations[243] = ram.allocate(allocSize);
    allocations[5] = ram.allocate(allocSize);
    allocations[123] = ram.allocate(allocSize);
    allocations[95] = ram.allocate(allocSize);
    allocations[151] = ram.allocate(allocSize * 4);

    EXPECT_NE(allocations[243].offset, miniram_t::NO_SPACE);
    EXPECT_NE(allocations[5].offset, miniram_t::NO_SPACE);
    EXPECT_NE(allocations[123].offset, miniram_t::NO_SPACE);
    EXPECT_NE(allocations[95].offset, miniram_t::NO_SPACE);
    EXPECT_NE(allocations[151].offset, miniram_t::NO_SPACE);

    for(uint32_t i = 0; i < numAllocs; i++)
        if(i < 152 || i > 154)
            ram.free(allocations[i]);

    EXPECT_EQ(ram.remaining(), 1024 * 1024 * 256);
    EXPECT_EQ(ram.max_alloc(), 1024 * 1024 * 256);

    auto validateAll = ram.allocate(1024 * 1024 * 256);
    EXPECT_EQ(validateAll.offset, 0);
}

MEM_TEST(miniram, SizeOfReporting) {
    miniram_t allocator(1024);
    EXPECT_EQ(allocator.size_of({.offset = miniram_t::NO_SPACE, .id = miniram_t::INVALID_INDEX}), 0);

    auto a = allocator.allocate(123);
    EXPECT_EQ(allocator.size_of(a), 123);

    auto b = allocator.allocate(456);
    EXPECT_EQ(allocator.size_of(b), 456);

    allocator.free(a);
    EXPECT_EQ(allocator.size_of(b), 456); // Size of b should be unaffected

    allocator.free(b);
}

MEM_TEST(miniram, FreeSpansReport) {
    miniram_t allocator(1024 * 10);
    auto a = allocator.allocate(100);
    auto b = allocator.allocate(200);
    auto c = allocator.allocate(300);
    auto d = allocator.allocate(400);

    allocator.free(b);
    allocator.free(d);

    auto const freeSpans = allocator.free_regions();

    // Test that we have exactly 2 non-empty bins
    size_t nonEmptyBins = 0;
    for(auto const& region : freeSpans) {
        if(region.count > 0) {
            nonEmptyBins++;
            // Each should have exactly 1 region
            EXPECT_EQ(region.count, 1);
        }
    }
    EXPECT_EQ(nonEmptyBins, 2);
}

MEM_TEST(miniram, OutOfSpace) {
    // Test failure by exhausting memory
    miniram_t allocator(1024);
    auto a = allocator.allocate(1024);
    EXPECT_NE(a.id, miniram_t::INVALID_INDEX);

    auto b = allocator.allocate(1); // Should fail, no space
    EXPECT_EQ(b.id, miniram_t::INVALID_INDEX);
    allocator.free(a);
}

MEM_TEST(miniram, OutOfNodes) {
    miniram_t ram(1024 * 1024, 5);
    std::vector<mini_alloc_t> allocations;
    for(int i = 0; i < 5; ++i) {
        allocations.push_back(ram.allocate(10));
        EXPECT_NE(allocations.back().id, miniram_t::INVALID_INDEX);
    }

    EXPECT_EQ(ram.remaining_allocs(), 0);

    auto oldAllocCapacity = ram.alloc_capacity();

    auto failure = ram.allocate(10); // Should not fail but extend
    EXPECT_NE(failure.id, miniram_t::INVALID_INDEX);

    EXPECT_TRUE(oldAllocCapacity < ram.alloc_capacity());
}

MEM_TEST(miniram, Reset) {
    miniram_t allocator(1024);
    allocator.allocate(256);
    allocator.allocate(512);

    EXPECT_EQ(allocator.remaining(), 1024 - 256 - 512);

    allocator.clear();

    EXPECT_EQ(allocator.remaining(), 1024);
    EXPECT_EQ(allocator.max_alloc(), 1024);

    auto a = allocator.allocate(1024);
    EXPECT_EQ(a.offset, 0);
    EXPECT_NE(a.id, miniram_t::INVALID_INDEX);
}

MEM_TEST(miniram, DisjunctCopyUtility) {
    std::vector<miniram_t::memmove_type> moves;
    // Case 1: A standard "move-left" with no overlap.
    moves.emplace_back(2000, 1000, 500);
    // Case 2: A "move-left" WITH overlap. dst (1400) + size (500) > src (1500).
    moves.emplace_back(1500, 1400, 500);

    auto disjunct = miniram_t::to_disjunct_copies(moves);

    // --- Verification ---
    // The first move was already disjunct, so it should be present as-is.
    EXPECT_EQ(disjunct[0].srcOffset, 2000);
    EXPECT_EQ(disjunct[0].dstOffset, 1000);
    EXPECT_EQ(disjunct[0].size, 500);

    // The second move (size 500) had to be split. The safe chunk size is src-dst = 100.
    // It should be split into 5 moves of 100 bytes each.
    EXPECT_EQ(disjunct.size(), 1 + 5); // 1 original move + 5 split moves

    // Check the first split chunk
    EXPECT_EQ(disjunct[1].srcOffset, 1500);
    EXPECT_EQ(disjunct[1].dstOffset, 1400);
    EXPECT_EQ(disjunct[1].size, 100);

    // Check the second split chunk
    EXPECT_EQ(disjunct[2].srcOffset, 1600);
    EXPECT_EQ(disjunct[2].dstOffset, 1500);
    EXPECT_EQ(disjunct[2].size, 100);

    // Check the last split chunk
    EXPECT_EQ(disjunct[5].srcOffset, 1900);
    EXPECT_EQ(disjunct[5].dstOffset, 1800);
    EXPECT_EQ(disjunct[5].size, 100);
}

MEM_TEST(miniram, DefragmentationAndMoveCoalescing) {
    miniram_t ram(10240); // 10 KB ram
    std::vector<mini_alloc_t> allocations;

    // Allocate 10 blocks of 500 bytes each
    for(int i = 0; i < 10; ++i) { allocations.push_back(ram.allocate(500)); }

    // Free blocks 0, 1, 4, 8, 9 to create holes and contiguous live sections
    ram.free(allocations[0]);
    ram.free(allocations[1]);
    ram.free(allocations[4]);
    ram.free(allocations[8]);
    ram.free(allocations[9]);

    // --- Perform Defragmentation ---
    auto report = ram.defragment();

    // --- Verification ---
    uint32_t const totalLiveSize = 5 * 500;
    uint32_t const totalFreeSize = ram.capacity() - totalLiveSize;
    EXPECT_EQ(ram.remaining(), totalFreeSize); // Total free space is correct.

    // The largest allocatable block should be close to the total free space
    // (within quantization error)
    EXPECT_GE(ram.max_alloc(), totalFreeSize - 100); // Allow small quantization difference
    EXPECT_LE(ram.max_alloc(), totalFreeSize);

    // Try to allocate the max available size
    auto largeAlloc = ram.allocate(ram.max_alloc());
    EXPECT_NE(largeAlloc.id, miniram_t::INVALID_INDEX);
    EXPECT_EQ(largeAlloc.offset, totalLiveSize);
}

MEM_TEST(miniheap, PostDefragIntegrity) {
    miniram_t ram(20480); // 20 KB heap
    std::vector<mini_alloc_t> allocations;

    // 1. SETUP: Create a fragmented heap
    // Allocate 20 blocks of 512 bytes each
    for(int i = 0; i < 20; ++i) { allocations.push_back(ram.allocate(512)); }

    // Keep track of which allocations will remain live
    std::map<miniram_t::index_type, mini_alloc_t> live_allocs;
    for(size_t i = 0; i < allocations.size(); ++i) {
        // Free even-numbered blocks, keep odd-numbered ones
        if(i % 2 == 0) { ram.free(allocations[i]); } else { live_allocs[allocations[i].id] = allocations[i]; }
    }
    // The heap is now a checkerboard of used/free blocks.

    // 2. DEFRAGMENT: Compact the heap
    auto report = ram.defragment();

    // In a real application, we would now update our pointers/handles.
    for(const auto& updated_alloc : report.updatedAllocs)
        live_allocs[updated_alloc.id].offset = updated_alloc.offset;

    // 3. POST-DEFRAG VALIDATION: Perform a mini-stress test on the compacted heap.

    // A. Trivial allocation from the new large free block
    auto a = ram.allocate(1024);
    EXPECT_NE(a.id, miniram_t::INVALID_INDEX);
    // It should be placed immediately after the 10 compacted live blocks.
    EXPECT_EQ(a.offset, 10 * 512);

    // B. Free an old (now moved) block and verify merging
    // Let's free the block that was originally allocations[5].
    auto alloc_to_free = live_allocs[allocations[5].id];
    ram.free(alloc_to_free);
    live_allocs.erase(allocations[5].id);

    // The max allocatable size should now be larger, as the freed block merged
    // with the main free area.
    uint32_t expected_free_space = ram.capacity() - (9 * 512) - 1024;
    EXPECT_EQ(ram.remaining(), expected_free_space);
    uint32_t min_expected = static_cast<uint32_t>(expected_free_space * 0.9);
    EXPECT_GE(ram.max_alloc(), min_expected);
    EXPECT_LE(ram.max_alloc(), expected_free_space);

    // C. Reuse the just-freed space (Best-Fit)
    // This allocation should reuse the 512-byte hole we just created.
    auto b = ram.allocate(500);
    EXPECT_NE(b.id, miniram_t::INVALID_INDEX);
    EXPECT_EQ(b.offset, alloc_to_free.offset); // Should be at the same offset as the freed block.

    // D. Final check: Free everything and ensure the heap is whole
    ram.free(a);
    ram.free(b);
    for(const auto& [metadata, alloc] : live_allocs) { ram.free(alloc); }

    EXPECT_EQ(ram.remaining(), ram.capacity());
    EXPECT_EQ(ram.max_alloc(), ram.capacity());

    auto validate_all = ram.allocate(ram.capacity());
    EXPECT_EQ(validate_all.offset, 0);
}

MEM_TEST(miniram, ResizeGrow) {
    miniram_t ram(1024);

    auto a = ram.allocate(100);
    auto b = ram.allocate(200);
    auto c = ram.allocate(300);

    ram.free(b); // Create fragmentation

    // Resize to 2048
    auto report = ram.resize(2048);

    EXPECT_EQ(ram.capacity(), 2048);
    EXPECT_EQ(ram.remaining(), 2048 - 100 - 300); // 1648 free
    EXPECT_EQ(report.moves.size(), 1); // Only 'c' moved to close gap
}

MEM_TEST(miniram, ResizeShrink) {
    miniram_t ram(2048);

    auto a = ram.allocate(100);
    auto b = ram.allocate(200);

    // Shrink to 1024 (used = 300, so 1024 is OK)
    auto report = ram.resize(1024);

    EXPECT_EQ(ram.capacity(), 1024);
    EXPECT_EQ(ram.remaining(), 1024 - 300); // 724 free
}
}

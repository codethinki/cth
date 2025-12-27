#include "cth/data/miniram.hpp"
#include "test.hpp"

#include <array>
#include <cstdint>
#include <map>
#include <random>
#include <vector>


namespace cth::dt {

DATA_TEST(miniram, basic_operations) {
    miniram allocator(1024 * 1024 * 256);

    auto a = allocator.allocate(1337);
    EXPECT_EQ(a.offset, 0);
    allocator.free(a);

    auto b = allocator.allocate(1337);
    EXPECT_EQ(b.offset, 0);
    allocator.free(b);

    auto validateAll = allocator.allocate(1024 * 1024 * 256);
    EXPECT_EQ(validateAll.offset, 0);
    allocator.free(validateAll);
}

DATA_TEST(miniram, sequential_allocations) {
    miniram allocator(1024 * 1024 * 256);

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

DATA_TEST(miniram, reuse_and_fragmentation) {
    miniram allocator(1024 * 1024 * 256);

    auto a = allocator.allocate(1024);
    EXPECT_EQ(a.offset, 0);
    auto b = allocator.allocate(3456);
    EXPECT_EQ(b.offset, 1024);

    allocator.free(a);

    auto c = allocator.allocate(1024);
    EXPECT_EQ(c.offset, 0);
    allocator.free(c);
    allocator.free(b);

    auto d = allocator.allocate(2345);
    EXPECT_EQ(d.offset, 0);
    auto e = allocator.allocate(456);
    EXPECT_EQ(e.offset, 2345);
    auto f = allocator.allocate(512);
    EXPECT_EQ(f.offset, 2345 + 456);

    EXPECT_EQ(allocator.remaining(), 1024 * 1024 * 256 - 2345 - 456 - 512);
    EXPECT_NE(allocator.max_alloc(), allocator.remaining());

    allocator.free(d);
    allocator.free(e);
    allocator.free(f);

    auto validateAll = allocator.allocate(1024 * 1024 * 256);
    EXPECT_EQ(validateAll.offset, 0);
}

DATA_TEST(miniram, zero_fragmentation_scenario) {
    miniram ram(1024 * 1024 * 256);
    constexpr uint32_t numAllocs = 256;
    constexpr uint32_t allocSize = 1024 * 1024;

    std::vector<mini_alloc> allocations(numAllocs);

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

    EXPECT_NE(allocations[243].offset, miniram::NO_SPACE);
    EXPECT_NE(allocations[5].offset, miniram::NO_SPACE);
    EXPECT_NE(allocations[123].offset, miniram::NO_SPACE);
    EXPECT_NE(allocations[95].offset, miniram::NO_SPACE);
    EXPECT_NE(allocations[151].offset, miniram::NO_SPACE);

    for(uint32_t i = 0; i < numAllocs; i++)
        if(i < 152 || i > 154)
            ram.free(allocations[i]);

    EXPECT_EQ(ram.remaining(), 1024 * 1024 * 256);
    EXPECT_EQ(ram.max_alloc(), 1024 * 1024 * 256);

    auto validateAll = ram.allocate(1024 * 1024 * 256);
    EXPECT_EQ(validateAll.offset, 0);
}

DATA_TEST(miniram, query_methods) {
    miniram allocator(1024);

    EXPECT_EQ(allocator.size_of({.offset = miniram::NO_SPACE, .id = miniram::INVALID_INDEX}), 0);

    auto a = allocator.allocate(123);
    EXPECT_EQ(allocator.size_of(a), 123);
    auto b = allocator.allocate(456);
    EXPECT_EQ(allocator.size_of(b), 456);

    allocator.free(a);
    EXPECT_EQ(allocator.size_of(b), 456);

    auto c = allocator.allocate(100);
    auto d = allocator.allocate(200);
    auto e = allocator.allocate(50);

    allocator.free(b);
    allocator.free(d);

    auto const freeSpans = allocator.free_regions();
    size_t nonEmptyBins = 0;
    for(auto const& region : freeSpans) {
        if(region.count > 0) {
            nonEmptyBins++;
            EXPECT_EQ(region.count, 1);
        }
    }
    EXPECT_EQ(nonEmptyBins, 2);
}

DATA_TEST(miniram, out_of_resources) {
    miniram allocator(1024);
    auto a = allocator.allocate(1024);
    EXPECT_NE(a.id, miniram::INVALID_INDEX);
    auto b = allocator.allocate(1);
    EXPECT_EQ(b.id, miniram::INVALID_INDEX);
    allocator.free(a);

    miniram ram(1024 * 1024, 5);
    std::vector<mini_alloc> allocations;
    for(int i = 0; i < 5; ++i) {
        allocations.push_back(ram.allocate(10));
        EXPECT_NE(allocations.back().id, miniram::INVALID_INDEX);
    }

    EXPECT_EQ(ram.remaining_allocs(), 0);
    auto const oldAllocCapacity = ram.alloc_capacity();
    auto [offset, id] = ram.allocate(10);
    EXPECT_NE(id, miniram::INVALID_INDEX);
    EXPECT_TRUE(oldAllocCapacity < ram.alloc_capacity());
}

DATA_TEST(miniram, reset) {
    miniram allocator(1024);
    [[maybe_unused]] auto _ = allocator.allocate(256);
    [[maybe_unused]] auto __ = allocator.allocate(512);

    EXPECT_EQ(allocator.remaining(), 1024 - 256 - 512);

    allocator.clear();

    EXPECT_EQ(allocator.remaining(), 1024);
    EXPECT_EQ(allocator.max_alloc(), 1024);

    auto a = allocator.allocate(1024);
    EXPECT_EQ(a.offset, 0);
    EXPECT_NE(a.id, miniram::INVALID_INDEX);
}

DATA_TEST(miniram, disjunct_copy_utility) {
    std::vector<miniram::memmove_type> moves;
    moves.emplace_back(2000, 1000, 500);
    moves.emplace_back(1500, 1400, 500);

    auto disjunct = miniram::to_disjunct_copies(moves);

    EXPECT_EQ(disjunct[0].srcOffset, 2000);
    EXPECT_EQ(disjunct[0].dstOffset, 1000);
    EXPECT_EQ(disjunct[0].size, 500);
    EXPECT_EQ(disjunct.size(), 1 + 5);
    EXPECT_EQ(disjunct[1].srcOffset, 1500);
    EXPECT_EQ(disjunct[1].dstOffset, 1400);
    EXPECT_EQ(disjunct[1].size, 100);
    EXPECT_EQ(disjunct[2].srcOffset, 1600);
    EXPECT_EQ(disjunct[2].dstOffset, 1500);
    EXPECT_EQ(disjunct[2].size, 100);
    EXPECT_EQ(disjunct[5].srcOffset, 1900);
    EXPECT_EQ(disjunct[5].dstOffset, 1800);
    EXPECT_EQ(disjunct[5].size, 100);
}

DATA_TEST(miniram, defragmentation) {
    miniram ram(10240);
    std::vector<mini_alloc> allocations;

    for(int i = 0; i < 10; ++i) { allocations.push_back(ram.allocate(500)); }

    ram.free(allocations[0]);
    ram.free(allocations[1]);
    ram.free(allocations[4]);
    ram.free(allocations[8]);
    ram.free(allocations[9]);

    auto report = ram.defragment();

    uint32_t const totalLiveSize = 5 * 500;
    uint32_t const totalFreeSize = ram.capacity() - totalLiveSize;
    EXPECT_EQ(ram.remaining(), totalFreeSize);
    EXPECT_GE(ram.max_alloc(), totalFreeSize - 100);
    EXPECT_LE(ram.max_alloc(), totalFreeSize);

    auto largeAlloc = ram.allocate(ram.max_alloc());
    EXPECT_NE(largeAlloc.id, miniram::INVALID_INDEX);
    EXPECT_EQ(largeAlloc.offset, totalLiveSize);
}

DATA_TEST(miniheap, post_defrag_integrity) {
    miniram ram(20480);
    std::vector<mini_alloc> allocations;

    for(int i = 0; i < 20; ++i) { allocations.push_back(ram.allocate(512)); }

    std::map<miniram::index_type, mini_alloc> liveAllocs;
    for(size_t i = 0; i < allocations.size(); ++i) {
        if(i % 2 == 0) { ram.free(allocations[i]); } else { liveAllocs[allocations[i].id] = allocations[i]; }
    }

    auto report = ram.defragment();

    for(const auto& updatedAlloc : report.updatedAllocs)
        liveAllocs[updatedAlloc.id].offset = updatedAlloc.offset;

    auto a = ram.allocate(1024);
    EXPECT_NE(a.id, miniram::INVALID_INDEX);
    EXPECT_EQ(a.offset, 10 * 512);

    auto allocToFree = liveAllocs[allocations[5].id];
    ram.free(allocToFree);
    liveAllocs.erase(allocations[5].id);

    uint32_t expectedFreeSpace = ram.capacity() - (9 * 512) - 1024;
    EXPECT_EQ(ram.remaining(), expectedFreeSpace);
    uint32_t minExpected = static_cast<uint32_t>(expectedFreeSpace * 0.9);
    EXPECT_GE(ram.max_alloc(), minExpected);
    EXPECT_LE(ram.max_alloc(), expectedFreeSpace);

    auto b = ram.allocate(500);
    EXPECT_NE(b.id, miniram::INVALID_INDEX);
    EXPECT_EQ(b.offset, allocToFree.offset);

    ram.free(a);
    ram.free(b);
    for(auto const& alloc : liveAllocs | std::views::values) { ram.free(alloc); }

    EXPECT_EQ(ram.remaining(), ram.capacity());
    EXPECT_EQ(ram.max_alloc(), ram.capacity());

    auto [offset, id] = ram.allocate(ram.capacity());
    EXPECT_EQ(offset, 0);
}

DATA_TEST(miniram, resize_operations) {
    miniram ram(1024);

    auto a = ram.allocate(100);
    auto b = ram.allocate(200);
    auto c = ram.allocate(300);

    ram.free(b);

    auto report = ram.resize(2048);

    EXPECT_EQ(ram.capacity(), 2048);
    EXPECT_EQ(ram.remaining(), 2048 - 100 - 300);
    EXPECT_EQ(report.moves.size(), 1);

    miniram ram2(1024);

    auto d = ram2.allocate(100);
    auto e = ram2.allocate(200);

    auto report2 = ram2.resize(1024);

    EXPECT_EQ(ram2.capacity(), 1024);
    EXPECT_EQ(ram2.remaining(), 1024 - 300);
}
}

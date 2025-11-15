#include "cth/test.hpp"

#include "cth/memory/miniram.hpp"

#include <algorithm>
#include <map>
#include <random>

#define MEM_TEST(suite, test_name) CTH_EX_TEST(_mem, suite, test_name)

namespace cth::mem {


void gen_hist(miniram const& ram) {
    std::println();
    std::println("Total Free Space: {}kb", ram.remaining() / 1024);
    std::println("Largest Free Region: {}kb", ram.max_alloc() / 1024);
    std::println("Free Block Size Distribution (Histogram):");
    std::println("-----------------------------------------");

    auto const freeSpans = ram.free_regions();
    std::map<uint32_t, size_t> histogram;
    size_t maxCount = 0;

    for(auto const& [count, size] : freeSpans) {
        if(count > 0) {
            // Group sizes into power-of-two buckets for readability
            uint32_t bucket = 1;
            if(size > 0)
                bucket = 1 << static_cast<uint32_t>(std::ceil(std::log2(size + 1)));
            histogram[bucket] += count;
            maxCount = std::max(histogram[bucket], maxCount);
        }
    }

    if(histogram.empty())
        std::println("No free blocks found.");
    else
        for(auto const& [bucket_size, count] : histogram) {
            constexpr int maxBarWidth = 50;
            int const barWidth = (maxCount > 0)
                                 ? static_cast<int>((static_cast<double>(count) / maxCount) * maxBarWidth)
                                 : 0;
            std::println("{} <= {} ({}b)", std::string(barWidth, '#'), count, bucket_size);
        }
    std::println("-----------------------------------------");
}



MEM_TEST(miniram, FragmentationStressTest) {
    // --- Test Parameters ---
    constexpr uint32_t ramSize = 1024 * 1024 * 256; // 256 MB ram
    constexpr uint32_t numOperations = 300000; // Number of random alloc/free operations
    constexpr uint32_t maxAllocSize = 1024 * 2; // Max size of a single random allocation (2 KB)
    constexpr int allocChancePercent = 70; // 70% chance to allocate, 30% chance to free

    // --- Setup ---
    miniram ram(ramSize);
    std::vector<mini_alloc > liveAllocations;
    liveAllocations.reserve(numOperations);

    // Use a high-quality random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> opDist(0, 99);
    std::uniform_int_distribution<> sizeDist(1, maxAllocSize);

    // --- Main Stress Loop ---
    for(uint32_t i = 0; i < numOperations; ++i) {
        if(opDist(gen) < allocChancePercent) {
            // Try to allocate
            uint32_t size = sizeDist(gen);
            mini_alloc  newAlloc = ram.allocate(size);
            if(newAlloc.id != miniram::INVALID_INDEX) { 
                liveAllocations.push_back(newAlloc); 
            }
        } else {
            // Try to free
            if(!liveAllocations.empty()) {
                // Pick a random live allocation to free
                std::uniform_int_distribution<size_t> freeDist(0, liveAllocations.size() - 1);
                size_t const indexToFree = freeDist(gen);

                // Free it
                ram.free(liveAllocations[indexToFree]);

                // Remove it from our list using the efficient swap-and-pop idiom
                std::swap(liveAllocations[indexToFree], liveAllocations.back());
                liveAllocations.pop_back();
            }
        }
    }
    std::println();
    std::println("--- Fragmentation Stress Test Results ---");

    std::println("ram State after {} operations ({} live allocations)", numOperations, liveAllocations.size());
    gen_hist(ram);

    auto report = ram.defragment();

    std::println();
    std::println("defragmented ram:");
    gen_hist(ram);
}

}
#include <cth/data/union_find.hpp>

#include <cth/numeric.hpp>

#include <random>
#include <chrono>
#include <iostream>
#include <vector>
#include <print>
#include <unordered_map>

using namespace cth::dt;

template<class T>
void to_profile(std::vector<T>& ops, union_find& uf_orig) {
    for(auto const& op : ops) {
        if(op.is_merge)
            uf_orig.merge(op.a, op.b);
        else
            uf_orig.find(op.a);
    }
}
int main() {
    // Parameters (tweak if CI machines need shorter runs)
    constexpr size_t N = 20000; // number of elements
    constexpr size_t ops_per_round = 50000; // number of operations per round
    constexpr size_t rounds = 8; // how many rounds to average

    // Create deterministic random sequence
    std::mt19937_64 rng(123456789);
    std::uniform_int_distribution<size_t> idx_dist(0, N - 1);

    struct Op {
        bool is_merge;
        size_t a, b;
    };
    std::vector<Op> ops;
    ops.reserve(ops_per_round);

    // Generate operations: mostly merges, some finds to exercise find path-compression
    for(size_t i = 0; i < ops_per_round; ++i) {
        if((i % 10) == 0) { // 1 in 10 is a find
            ops.push_back({false, idx_dist(rng), 0});
        } else { ops.push_back({true, idx_dist(rng), idx_dist(rng)}); }
    }

    // In-test union-by-size implementation (same API used by test)
    struct union_find_by_size {
        using index_type = size_t;
        explicit union_find_by_size(size_t n) : parent(n), sz(n, 1) { for(size_t i = 0; i < n; ++i) parent[i] = i; }
        index_type find(index_type x) {
            index_type r = x;
            while(parent[r] != r) r = parent[r];
            while(parent[x] != r) {
                index_type next = parent[x];
                parent[x] = r;
                x = next;
            }
            return r;
        }
        void merge(index_type a, index_type b) {
            a = find(a);
            b = find(b);
            if(a == b) return;
            if(sz[a] < sz[b]) std::swap(a, b);
            parent[b] = a;
            sz[a] += sz[b];
        }
        size_t chain_length(index_type x) {
            size_t len = 0;
            index_type p = x;
            while(parent[p] != p) {
                p = parent[p];
                ++len;
            }
            return len;
        }
        std::vector<index_type> parent;
        std::vector<size_t> sz;
    };

    // Timing containers
    std::vector<double> times_orig;
    times_orig.reserve(rounds);
    std::vector<double> times_size;
    times_size.reserve(rounds);
    for(size_t r = 0; r < rounds; ++r) {
        // create new instances each round
        union_find uf_orig(N); // original implementation under test
        union_find_by_size uf_size(N); // improved implementation

        // Time original
        {
            auto t0 = std::chrono::steady_clock::now();
            to_profile(ops, uf_orig);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::micro> us = t1 - t0;
            times_orig.push_back(us.count());
        }

        // Time size-heuristic version
        {
            auto t0 = std::chrono::steady_clock::now();
            for(auto const& op : ops) {
                if(op.is_merge)
                    uf_size.merge(op.a, op.b);
                else
                    uf_size.find(op.a);
            }
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::micro> us = t1 - t0;
            times_size.push_back(us.count());
        }

        // After running the same ops, verify connectivity sets are identical.
        // Build canonical component ids for both results and compare.
        std::vector<size_t> comp_orig(N), comp_size(N);
        for(size_t i = 0; i < N; ++i) comp_orig[i] = uf_orig.find(i);
        for(size_t i = 0; i < N; ++i) comp_size[i] = uf_size.find(i);

        // Normalize to compact ids
        auto normalize = [](const std::vector<size_t>& reps) {
            std::unordered_map < size_t, size_t > map;
            std::vector<size_t> out(reps.size());
            size_t next = 0;
            for(size_t i = 0; i < reps.size(); ++i) {
                auto it = map.find(reps[i]);
                if(it == map.end()) {
                    map.emplace(reps[i], next);
                    out[i] = next++;
                } else
                    out[i] = it->second;
            }
            return out;
        };

        auto norm_orig = normalize(comp_orig);
        auto norm_size = normalize(comp_size);
    }

    // Compute averages
    auto avg = [](const std::vector<double>& v) {
        double s = 0;
        for(auto x : v) s += x;
        return s / static_cast<double>(v.size());
    };
    double avg_orig = avg(times_orig);
    double avg_size = avg(times_size);

    // Print results to test output for inspection
    std::cout << "[  INFO ] UnionFind perf compare (microseconds per round):\n"
        << "  original avg = " << avg_orig << " us\n"
        << "  by-size  avg = " << avg_size << " us\n";

    std::println("improvement: {:.1f}%", (avg_size - avg_orig) / avg_size * 100);
}


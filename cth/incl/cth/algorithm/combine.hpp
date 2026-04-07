#pragma once
#include "cth/io/log.hpp"
#include "cth/meta/ranges.hpp"

#include <algorithm>
#include <concepts>
#include <numeric>
#include <queue>
#include <ranges>
#include <vector>
#include <unordered_map>
#include <unordered_set>



/**
 * @brief creates a vector of unique elements with exactly one element from each given range. order is
 *  preserved
 * @tparam Rng rng<rng<std::equality_comparable>>
 * @param selections range of ranges
 * @return vector of T, empty if no unique selection is possible
 */
namespace cth::alg {

    template<mta::range2d_over_cpt<CPT(std::equality_comparable)> Rng>
    [[nodiscard]] auto unique_combine(Rng const& selections) {
        using T = std::remove_cvref_t<mta::range2d_value_t<Rng>>;
        using node_t = T const*;
        using node_selection_map_t = std::unordered_map<node_t, size_t>;
        static constexpr node_t EMPTY_NODE = nullptr;

        auto const selectionCount = std::ranges::size(selections);

        // 1. Stable Memory Pool
        // Pointers to elements in an unordered_set are strictly stable across rehashes.
        std::unordered_set<T> pool{};

        // Graph state now relies purely on raw memory addresses

        node_selection_map_t nodeToSelection{};
        std::vector<node_t> selectionToNode(selectionCount, EMPTY_NODE);

        // 2. Isolated BFS Engine
        auto find_augmenting_path = [&](size_t start, node_selection_map_t& parent) -> node_t {
            std::queue<size_t> q{{start}};
            while(!q.empty()) {
                auto current = q.front();
                q.pop();

                for(auto const& val : selections[current]) {
                    // insert, ignores duplicates
                    auto node = &(*pool.insert(val).first);

                    // try_emplace avoids cycles by rejecting if ptr is already in parent
                    if(!parent.try_emplace(node, current).second)
                        continue;

                    if(!nodeToSelection.contains(node)) // check claimed
                        return node;

                    q.push(nodeToSelection[node]); // claimed -> move owner
                }
            }
            return EMPTY_NODE;
        };

        // 3. Main Logic
        node_selection_map_t parent{};

        for(size_t i = 0; i < selectionCount; ++i) {
            auto end = find_augmenting_path(i, parent);
            if(!end) return std::vector<T>{}; // Dead end

            // Backtrack and flip assignments using stable pointers
            for(auto current = end; current != nullptr;) {
                auto right = parent[current];
                auto previous = selectionToNode[right];

                selectionToNode[right] = current;
                nodeToSelection[current] = right;

                current = previous; // Move up the path
            }

            parent.clear();
        }

        // 4. Format Output
        std::vector<T> out;
        out.reserve(selectionCount);
        for(auto ptr : selectionToNode)
            out.push_back(std::move(pool.extract(*ptr).value()));

        return out;
    }

}



namespace cth::alg {
    //TEMP this algorithm is trash and can be written as a gold-plated version of the above
/**
 * \brief assigns a's to b's based of every a's options for b's
 * \tparam Rng1 rng<rng<std::integral>>
 * \tparam Rng2 rng<std::integral>
 * \param a_b_options range of b options from a's
 * \param b_max  max assignments for b's
 * \return vector<integral> based on Rng1
 */
template<mta::range2d_over_cpt<CPT(std::integral)> Rng1, mta::range_over_cpt<CPT(std::integral)> Rng2>
auto assign(Rng1 const& a_b_options, Rng2 const& b_max) -> std::vector<mta::range2d_value_t<Rng1>> {
    using T = mta::range2d_value_t<Rng1>;

    CTH_CRITICAL(
        !std::ranges::all_of(
            a_b_options,
            [&b_max](std::span<T const> b_options) {
            return std::ranges::all_of(b_options, [&b_max](auto const index) {
                return 0 <= index && index < std::ranges::size(b_max);
                });
            }
        ),
        "0 <= indices < size(b_max) required"
    ) {
        details->add("size(b_max): {}", std::ranges::size(b_max));
    }


    std::vector<T> bOptionsIndices(std::ranges::size(a_b_options));
    std::ranges::iota(bOptionsIndices, 0);

    auto valid = [&a_b_options, &b_max](std::span<T const> b_options_indices) {
        std::vector<T> assignments(std::ranges::size(b_max), 0);
        std::vector<T> combination(std::ranges::size(a_b_options), 0);

        for(size_t i = 0; i < std::ranges::size(a_b_options); i++) {
            auto const& bOptions = a_b_options[b_options_indices[i]];

            for(T const b : bOptions) {
                if(assignments[b] >= b_max[b])
                    continue;
                ++assignments[b];
                combination[b_options_indices[i]] = b;
                goto next;
            }
            return std::vector<T>{};
        next:;
        }


        return combination;
    };


    do {
        auto const& result = valid(bOptionsIndices);
        if(!result.empty())
            return result;
    } while(std::ranges::next_permutation(bOptionsIndices).found);


    return std::vector<T>{};
}

} // namespace cth::alg

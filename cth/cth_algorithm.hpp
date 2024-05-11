#pragma once
#include <algorithm>
#include <numeric>
#include <ranges>
#include <unordered_set>


#include "cth_log.hpp"
#include "cth_type_traits.hpp"


namespace cth::algorithm::hash {
// from: https://stackoverflow.com/a/57595105
template<typename T, typename... Rest>
void combine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (cth::algorithm::hash::combine(seed, rest), ...);
}

} // namespace cth::algorithm::hash

namespace cth::algorithm {

/**
* \brief creates a vector of unique elements with exactly one element from each given range. order is preserved
* \tparam Rng rng<rng<std::equality_comparable>>
* \param selections range of ranges
* \return vector of T, empty if no unique selection is possible
* \note this is bruteforce
*/
template<type::range2d_over_cpt<cpt(std::equality_comparable)> Rng>
[[nodiscard]] auto uniqueCombine(const Rng& selections) {
    using T = std::remove_cvref_t<type::range2d_value_t<Rng>>;


    std::unordered_set<T> uniqueElements{};
    std::vector<std::size_t> indexIndices(std::size(selections));

    auto i = 0u;
    while(indexIndices[0] < std::size(selections[0])) {
        for(; i < indexIndices.size(); i++) {
            for(; uniqueElements.contains(selections[i][indexIndices[i]]); ++indexIndices[i])
                if(indexIndices[i] >= std::size(selections[i]) - 1) goto notFinished;
            uniqueElements.insert(selections[i][indexIndices[i]]);
        }
        break; //finished
    notFinished:
        indexIndices[i] = 0;
        for(; ++indexIndices[i - 1] >= std::size(selections[i - 1]); --i) {
            uniqueElements.erase(selections[i - 1][indexIndices[i - 1] - 1]);
            if(i == 1) return std::vector<T>{};
            indexIndices[i - 1] = 0;
        }
    }
    std::vector<T> out{};
    out.reserve(std::size(selections));

    for(auto [index, selection] : std::views::zip(indexIndices, selections))
        out.push_back(selection[index]);

    return out;
}

/**
 * \brief assigns a to b based of a's options for b's
 * \tparam Rng1 rng<rng<std::integral>>
 * \tparam Rng2 rng<std::integral>
 * \param a_b_options range of b options from a's
 * \param b_max  max assignments for b's
 * \return vector<integral> based on Rng1
 */
template<type::range2d_over_cpt<cpt(std::integral)> Rng1, type::range_over_cpt<cpt(std::integral)> Rng2>
auto assign(const Rng1& a_b_options, const Rng2& b_max)->std::vector<type::range2d_value_t<Rng1>> {
    using T = type::range2d_value_t<Rng1>;

    CTH_ERR(std::ranges::any_of(a_b_options, [b_max](const std::span<const T> b_options) {
        return std::ranges::any_of(b_options, [b_max](const auto index){ return 0 <= index && index < std::ranges::size(b_max); });
        }), "0 <= indices < size(b_max) required");


    std::vector<T> bOptionsIndices(std::ranges::size(a_b_options));
    std::ranges::iota(bOptionsIndices, 0);

    auto valid = [a_b_options, b_max](const std::span<const T> b_options_indices) {
        const auto aBOptions = b_options_indices | std::views::transform([a_b_options](const T index) { return a_b_options[index]; });

        std::vector<T> assignments(std::ranges::size(b_max), 0);
        std::vector<T> combination(std::ranges::size(b_max), 0);

        for(auto [bOptions, result] : std::views::zip(aBOptions, combination)) {
            for(const T b : bOptions) {
                if(assignments[b] >= b_max[b]) continue;
                ++assignments[b];
                result = b;
                goto next;
            }
            return std::vector<T>{};
        next:;
        }


        return combination;
    };


    while(std::ranges::next_permutation(bOptionsIndices).found) {
        auto result = valid(bOptionsIndices);
        if(!result.empty()) return result;
    }
    return {};
}


} // namespace cth::algorithm

module;

#include "../macro.hpp"
#include "../io/io_log.hpp"

export module cth.alg.ranges;

import cth.typ.ranges;
import cth.io.log;

import std;

export namespace cth::alg {

/**
* \brief creates a vector of unique elements with exactly one element from each given range. order is preserved
* \tparam Rng rng<rng<std::equality_comparable>>
* \param selections range of ranges
* \return vector of T, empty if no unique selection is possible
* \note this is bruteforce
*/
template<cth::type::range2d_over_cpt<CPT(std::equality_comparable)> Rng>
[[nodiscard]] auto uniqueCombine(Rng const& selections) {
    using T = std::remove_cvref_t<type::range2d_value_t<Rng>>;


    std::unordered_set<T> uniqueElements{};
    std::vector<std::size_t> indexIndices(std::size(selections));

    auto i = 0u;
    while(indexIndices[0] < std::ranges::size(selections[0])) {
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
 * \brief assigns a's to b's based of every a's options for b's
 * \tparam Rng1 rng<rng<std::integral>>
 * \tparam Rng2 rng<std::integral>
 * \param a_b_options range of b options from a's
 * \param b_max  max assignments for b's
 * \return vector<integral> based on Rng1
 */
template<type::range2d_over_cpt<CPT(std::integral)> Rng1, type::range_over_cpt<CPT(std::integral)> Rng2>
auto assign(Rng1 const& a_b_options, Rng2 const& b_max) -> std::vector<type::range2d_value_t<Rng1>> {
    using T = type::range2d_value_t<Rng1>;

    CTH_CRITICAL(!std::ranges::all_of(a_b_options, [&b_max](std::span<T const> b_options) {
        return std::ranges::all_of(b_options, [&b_max](auto const index){ return 0 <= index && index < std::ranges::size(b_max); });
        }), "0 <= indices < size(b_max) required") { details->add("size(b_max): {}", std::ranges::size(b_max)); }


    std::vector<T> bOptionsIndices(std::ranges::size(a_b_options));
    std::ranges::iota(bOptionsIndices, 0);

    auto valid = [&a_b_options, &b_max](std::span<T const> b_options_indices) {
        std::vector<T> assignments(std::ranges::size(b_max), 0);
        std::vector<T> combination(std::ranges::size(a_b_options), 0);

        for(size_t i = 0; i < std::ranges::size(a_b_options); i++) {
            auto const& bOptions = a_b_options[b_options_indices[i]];

            for(T const b : bOptions) {
                if(assignments[b] >= b_max[b]) continue;
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
        if(!result.empty()) return result;
    } while(std::ranges::next_permutation(bOptionsIndices).found);


    return std::vector<T>{};
}


} // namespace cth::alg
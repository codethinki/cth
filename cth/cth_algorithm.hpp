#pragma once
#include <algorithm>
#include <unordered_set>

#include "cth_log.hpp"



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
* \brief creates a vector of unique elements with exactly one element from each given selection. order is preserved
* \tparam Rng rng<rng<T>> with T comparable type
* \param selections vector of selections for every index
*/
template<typename Rng>
auto uniqueSelect(const Rng& selections) {
    using T = std::decay_t<decltype(*std::begin(*std::begin(selections)))>;


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
} // namespace cth::algorithm

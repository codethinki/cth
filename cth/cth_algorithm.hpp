#pragma once
#include <algorithm>

#include "cth_concepts.hpp"
#include "cth_log.hpp"

//TEMP untested

namespace cth::algorithm {
template<typename It, type::unsigned_t Size, type::arithmetic_t T>
constexpr void fillInc(It first, Size count, T increment, T start = 0) { for(Size i = 0; i < count; ++i) *first++ = start += increment; }
} // namespace cth::algorithm



namespace cth::algorithm::unsorted {
using namespace std;

template<typename BidIt>
constexpr void doubleSelectionSort(BidIt begin, BidIt end) {
    if(begin == end) return;

    BidIt min = begin;
    BidIt max = end;
    --max;

    while(begin < max) {
        min = begin;
        max = begin;
        for(BidIt it = begin; it != end; ++it) {
            if(*it < *min) min = it;
            if(*it > *max) max = it;
        }
        std::iter_swap(begin, min);
        if(max == begin) max = min;
        std::iter_swap(max, end - 1);
        ++begin;
        --end;
    }
}

} // namespace cth::algorithm::unsorted


namespace cth::expr::algorithm {
template<typename It, type::unsigned_t Size, type::arithmetic_t T>
constexpr void fillInc(It first, Size count, T increment, T start = 0) { cth::algorithm::fillInc(first, count, increment, start); }


namespace unsorted {
    template<typename BidIt>
    constexpr void doubleSelectionSort(BidIt begin, BidIt end) { algorithm::unsorted::doubleSelectionSort(begin, end); }
}
}

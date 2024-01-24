#pragma once
#include <algorithm>

#include "src/cth_concepts.hpp"
#include "src/cth_log.hpp"

//TEMP untested

namespace cth::algorithm {
template<typename It, unsigned_t Size, arithmetic_t T>
constexpr void fillInc(It first, Size count, T increment, T start = 0) { for(Size i = 0; i < count; ++i) *first++ = start += increment; }
} // namespace cth::algorithm



namespace cth::algorithm::unsorted {
using namespace std;

template<typename BidIt>
constexpr void doubleSelectionSort(BidIt begin, BidIt end) {
    //TEMP not working fix it

    while(begin != end--) {
        BidIt smallest = begin;
        BidIt largest = end;
        for(BidIt k = begin; k != end; ++k) {
            if(*smallest > *k) smallest = k;
            if(*largest < *k) largest = k;
        }
        if(smallest != begin) std::iter_swap(begin, smallest);
        if(largest != end) std::iter_swap(end, largest);

        ++begin;
    }
}

}


namespace cth::expr::algorithm {
template<typename It, unsigned_t Size, arithmetic_t T>
constexpr void fillInc(It first, Size count, T increment, T start = 0) { cth::algorithm::fillInc(first, count, increment, start); }


namespace unsorted {
    template<typename BidIt>
    constexpr void doubleSelectionSort(BidIt begin, BidIt end) { algorithm::unsorted::doubleSelectionSort(begin, end); }
}
}

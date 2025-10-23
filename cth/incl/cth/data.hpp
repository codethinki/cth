#pragma once
#include "types/typ_traits.hpp"

#include <algorithm>
#include <bit>
#include <numeric>
#include <span>
#include <vector>

#include <memory>

//namespace cth::dt {
//class union_find {
//
//
//public:
//    using index_type = size_t;
//    using size_type = uint32_t;
//
//private:
//    template<class S>
//    [[nodiscard]] constexpr auto& size(this S& self, index_type x) {
//        return *(reinterpret_cast<type::fwd_const_t<S, size_type>*>(&self.parent(self._size)) + x);
//    }
//
//public:
//    constexpr explicit union_find(size_t n) :
//        _data(n * sizeof(index_type) + n * sizeof(size_type)), _size{n} {
//        for(size_t i = 0; i < n; i++) {
//            parent(i) = i;
//            size(i) = 1;
//        }
//    }
//
//
//
//    constexpr index_type find(this union_find& self, index_type x) {
//        auto r = x;
//
//        do {
//            self.parent(r) = self.parent(self.parent(r));
//            r = self.parent(r);
//        } while(!self.root(r));
//
//        return r;
//    }
//
//    constexpr void merge(this union_find& self, index_type a, index_type b) {
//        auto const ra = self.find(a);
//        auto const rb = self.find(b);
//
//        if(ra == rb) return;
//
//        auto const& [p, c] = self.size(ra) > self.size(rb) ? std::pair{ra, rb} : std::pair{rb, ra};
//
//        self.parent(c) = p;
//        self.size(p) += 1;
//    }
//
//
//    constexpr size_t chain_length(this auto const& self, index_type x) {
//        size_t len = 0;
//        auto p = x;
//        for(; !self.root(p); len++) p = self.parent(p);
//
//        return len;
//    }
//
//private:
//    template<class S>
//    [[nodiscard]] constexpr auto& parent(this S& s, index_type x) {
//        return *(reinterpret_cast<type::fwd_const_t<S, index_type>*>(s._data.data()) + x);
//    }
//
//    std::vector<std::byte> _data;
//    size_t _size;
//
//public:
//    [[nodiscard]] constexpr bool root(this auto const& self, index_type x) { return self.parent(x) == x; }
//    [[nodiscard]] constexpr size_t size(this auto const& self) { return self._size; }
//
//};
//}




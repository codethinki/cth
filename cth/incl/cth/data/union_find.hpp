#pragma once
#include "cth/data/poly_vector.hpp"


namespace cth::dt {
class union_find {
public:
    using index_type = size_t;
    using size_type = uint32_t;

private:
    [[nodiscard]] auto& size(this auto& s, index_type x) { return s._data.template data<1>()[x]; }

    template<class S>
    [[nodiscard]] auto& parent(this S& s, index_type x) { return s._data.template data<0>()[x]; }

public:
    explicit union_find(size_t n) : _data({n, n}), _size{n} {
        for(index_type i = 0; i < n; i++) {
            parent(i) = i;
            size(i) = 1;
        }
    }


    [[nodiscard]] index_type find(this union_find& self, index_type x) {
        auto r = x;
        do {
            auto& p = self.parent(r);
            p = self.parent(p);
            r = p;
        }
        while(!self.root(r));

        return r;
    }

    void merge(this union_find& self, index_type a, index_type b) {
        auto child = self.find(a);
        auto parent = self.find(b);

        if(child == parent) return;

        if(parent < child) std::swap(child, parent);

        self.parent(child) = parent;
        self.size(parent) += self.size(child);
    }


    [[nodiscard]] size_t chain_length(this auto const& self, index_type x) {
        size_t len = 0;
        auto p = x;
        for(; !self.root(p); len++) p = self.parent(p);

        return len;
    }

private:
    raw_poly_vector<index_type, size_type> _data;

    size_t _size;

public:
    [[nodiscard]] bool root(this auto const& self, index_type x) { return self.parent(x) == x; }
    [[nodiscard]] size_t size(this auto const& self) { return self._size; }
};
} // namespace cth::dt

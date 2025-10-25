#pragma once
#include "cth/data/poly_vector.hpp"

#include <unordered_set>

namespace cth::dt {
class union_find {
public:
    using index_type = size_t;
    using size_type = uint32_t;
    using group_t = std::unordered_set<index_type>;

private:
    [[nodiscard]] constexpr auto& size(this auto& s, index_type x) { return s._data.template data<1>()[x]; }

    template<class S>
    [[nodiscard]] constexpr auto& parent(this S& s, index_type x) { return s._data.template data<0>()[x]; }

public:
    /**
     * constructs with given size
     */
    explicit constexpr union_find(size_t size) : _data({size, size}), _size{size} {
        for(index_type i = 0; i < size; i++) {
            parent(i) = i;
            this->size(i) = 1;
        }
    }

    /**
     * constructs and immediately merges given pairs
     * @param n size
     * @param groups to execute immediately
     * @details calls @ref union_find(size_t)
     */
    union_find(size_t n, std::span<group_t const> groups) : union_find{n} {
        for(auto& group : groups)
            merge(group);
    }

    /**
     * delegation for @ref union_find(size_t, std::span<merge_t const>)
     */
    union_find(size_t n, std::initializer_list<group_t> groups) : union_find{n, std::span{groups}} {}

    // ReSharper disable once CppNonExplicitConvertingConstructor
    union_find(std::span<group_t const> groups) : union_find(std::ranges::max(groups | std::views::join) + 1, groups) {}

    union_find(std::initializer_list<group_t> groups) : union_find(std::span{groups}) {}

    /**
     * finds the root of @ref x, uses path compression
     * @return root index
     */
    [[nodiscard]] constexpr index_type find(this union_find& self, index_type x) {
        auto r = x;
        do {
            auto& p = self.parent(r);
            p = self.parent(p);
            r = p;
        }
        while(!self.root(r));

        return r;
    }
    /**
     * finds the root of @ref x, does NOT use path compression
     * @return root index
     */
    [[nodiscard]] constexpr index_type find(this union_find const& self, index_type x) {
        auto r = x;
        do { r = self.parent(r); }
        while(!self.root(r));

        return r;
    }

    /**
     * merges the sets of a and b
     */
    constexpr void merge(this union_find& self, index_type lhs, index_type rhs) {
        auto child = self.find(lhs);
        auto parent = self.find(rhs);

        if(child == parent) return;

        if(parent < child) std::swap(child, parent);

        self.parent(child) = parent;
        self.size(parent) += self.size(child);
    }
    /**
     * delegates to @ref merge(index_type, index_type)
     * @param group merge pair
     */
    void merge(group_t const& group) {
        if(group.empty()) return;
        auto const root = find(*group.begin());
        auto& rootSize = size(root);

        for(auto& node : group) {
            auto& p = parent(node);

            if(p == root) continue;

            rootSize++;
            p = root;
        }
    }


    [[nodiscard]] constexpr size_t chain_length(this auto const& self, index_type x) {
        size_t len = 0;
        auto p = x;
        for(; !self.root(p); len++) p = self.parent(p);

        return len;
    }

    /**
     * finds all root indices
     * @return root indices
     */
    [[nodiscard]] constexpr std::vector<index_type> roots(this auto const& self) {
        std::vector<index_type> res{};
        res.reserve(self._size);
        for(index_type i = 0; i < self._size; i++)
            if(self.root(i))
                res.push_back(i);

        return res;
    }

private:
    poly_vector<index_type, size_type> _data;

    size_t _size;

public:
    /**
     * checks if @ref x is a root
     */
    [[nodiscard]] constexpr bool root(this auto const& self, index_type x) { return self.parent(x) == x; }
    /**
     * size of the union find
     */
    [[nodiscard]] constexpr size_t size(this auto const& self) { return self._size; }
};
} // namespace cth::dt

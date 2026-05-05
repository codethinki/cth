#pragma once
#include "cth/data/poly_vector.hpp"

#include <unordered_set>

namespace cth::dt {
/**
 * union find data structure implementation
 * @pre not thread safe
 */
class union_find {
public:
    using index_type = size_t;
    using size_type = uint32_t;
    using group_t = std::vector<index_type>;

private:
    [[nodiscard]] constexpr auto& size_ref(this auto& s, index_type x) { return s._data.template data<1>()[x]; }

    template<class S>
    [[nodiscard]] constexpr auto& parent(this S& s, index_type x) { return s._data.template data<0>()[x]; }

public:
    /**
     * constructs with given size
     */
    explicit constexpr union_find(size_t size) : _data({size, size}), _size{size} {
        for(index_type i = 0; i < size; i++) {
            parent(i) = i;
            this->size_ref(i) = 1;
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
    union_find(std::span<group_t const> groups) : union_find(
        std::ranges::max(groups | std::views::join) + 1,
        groups
    ) {}

    union_find(std::initializer_list<group_t> groups) : union_find(std::span{groups}) {}

    /**
     * finds the root of @ref x, uses path compression
     * @param x must be in `[0, size)`
     * @return root index
     */
    [[nodiscard]] constexpr index_type find(index_type x) const {
        check_bounds(x);
        auto r = x;
        do {
            auto& p = parent(r);
            p = parent(p);
            r = p;
        }
        while(!root(r));

        return r;
    }

    /**
     * merges the sets of a and b
     * @param lhs must be in bounds `[0, size)`
     * @param rhs must be in bounds `[0, size)`
     */
    constexpr void merge(this union_find& s, index_type lhs, index_type rhs) {
        auto child = s.find(lhs);
        auto parent = s.find(rhs);

        if(child == parent)
            return;

        if(parent < child)
            std::swap(child, parent);

        s.parent(child) = parent;
        s.size_ref(parent) += s.size_ref(child);
    }
    /**
     * delegates to @ref merge(index_type, index_type)
     * @param group set of indices to merge
     * @pre all of `group` in bounds `[0, size)`
     * @param group merge pair
     */
    void merge(group_t const& group) {
        if(group.empty())
            return;
        auto const root = find(*group.begin());
        auto& rootSize = size_ref(root);

        for(auto& node : group) {
            auto& p = parent(node);

            if(p == root)
                continue;

            rootSize++;
            p = root;
        }
    }

    [[nodiscard]] constexpr size_t chain_length(index_type x) const {
        size_t len = 0;
        auto p = x;
        for(; !root(p); len++)
            p = parent(p);

        return len;
    }

    /**
     * finds all root indices
     * @details O(N)
     * @return root indices
     */
    [[nodiscard]] constexpr std::vector<index_type> roots() const {
        std::vector<index_type> res{};
        for(index_type i = 0; i < size(); i++)
            if(root(i))
                res.push_back(i);

        return res;
    }


    /**
     * finds all indices with in the same group
     * @pre `x` in bounds `[0, size)`
     * @param x to find group members of
     */
    [[nodiscard]] constexpr std::vector<index_type> all_of(index_type x) {
        check_bounds(x);


        std::vector<index_type> result{};
        auto const parent = find(x);

        for(index_type i = 0; i < size(); i++)
            if(find(i) == parent)
                result.push_back(i);

        return result;
    }

    [[nodiscard]] constexpr std::vector<group_t> all() {
        std::vector<group_t> result{};

        std::unordered_map<index_type, size_t> groups{};

        for(index_type idx = 0; idx < size(); idx++) {
            auto const parent = find(idx);
            if(auto groupIt = groups.find(parent); groupIt != groups.end())
                result[groupIt->second].emplace_back(idx);
            else {
                groups[parent] = result.size();
                result.emplace_back(1)[0] = idx;
            }
        }
        return result;
    }

private:
    constexpr void check_bounds(this union_find const& self, index_type x) {
        CTH_CRITICAL(x >= self.size(), "index({}) out of bounds for [0, {})", x, self.size()) {}
    }

    mutable poly_vector<index_type, size_type> _data;

    size_t _size;

public:
    bool operator==(union_find const& other) const {
        if(size() != other.size())
            return false;
        auto size = static_cast<index_type>(this->size());
        for(index_type i = 0; i < size; i++)
            if(find(i) != other.find(i))
                return false;

        return true;
    }

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

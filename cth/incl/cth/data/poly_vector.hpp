#pragma once
#include "cth/types/typ_concepts.hpp"
#include "cth/types/typ_traits.hpp"

#include "cth/io/log.hpp"

#include <array>
#include <span>
#include <vector>


namespace cth::dt {

template<type::trivial... Ts>
class raw_poly_vector {
public:
    static constexpr size_t SIZE = sizeof...(Ts);
    static constexpr size_t N = sizeof...(Ts);

    template<size_t I> requires(I < N)
    using value_type = type::get_t<I, Ts...>;

    explicit raw_poly_vector(std::span<size_t const> sizes) {
        CTH_CRITICAL(sizes.size() != N, "invalid size range given") {}

        constexpr std::array<size_t, N> typeSizes{sizeof(Ts)...};
        constexpr std::array<size_t, N> typeAligns{alignof(Ts)...};
        std::array<size_t, N + 1> offsets{};

        size_t currentOffset = 0;

        for(size_t i = 0; i < N; ++i) {
            auto const alignment = typeAligns[i];

            // Calculate padding required to align the current offset
            auto const padding = (alignment - (currentOffset % alignment)) % alignment;
            currentOffset += padding;

            // Store the aligned offset for this type
            offsets[i] = currentOffset;

            // Advance the offset by the size of this array
            currentOffset += sizes[i] * typeSizes[i];
        }

        _data.resize(currentOffset);

        for(size_t i = 0; i < N - 1; ++i)
            _begins[i] = &_data[offsets[i + 1]];
    }

    explicit raw_poly_vector(std::array<size_t, N> const& sizes) : raw_poly_vector(std::span{sizes}) {}

    explicit raw_poly_vector(std::initializer_list<size_t> sizes) : raw_poly_vector(std::span{sizes.begin(), sizes.size()}) {}

    template<size_t I, class S> requires(I < N)
    [[nodiscard]] auto data(this S& s) noexcept {
        using ptr_t = type::fwd_const_t<S, value_type<I>>*;
        if constexpr(I == 0)
            return reinterpret_cast<ptr_t>(s._data.data());
        else
            // TEMP make constexpr, use std::start_lifetime_as_array
            return static_cast<ptr_t>(s._begins[I - 1]);
    }

private:
    std::vector<std::byte> _data;
    std::array<void*, N - 1> _begins{};
};


template<type::trivial... Ts>
class poly_vector : raw_poly_vector<size_t, Ts...> {
    using base_t = raw_poly_vector<size_t, Ts...>;

    template<class S>
    using cbase_t = type::fwd_const_t<S, base_t>;

    template<class S>
    [[nodiscard]] auto* sizes(this S& s) noexcept {
        // sizes are stored in the first block (index 0) of the raw buffer
        return s.base_t::template data<0, cbase_t<S>>();
    }

public:
    static constexpr size_t SIZE = sizeof...(Ts);
    static constexpr size_t N = sizeof...(Ts);

    template<size_t I> requires(I < N)
    using value_type = type::get_t<I, Ts...>;

    explicit poly_vector(std::span<size_t const> sz) : base_t(make_sizes_array(sz)) {
        CTH_CRITICAL(sz.size() != N, "invalid size range given") {}
        for(size_t i = 0; i < N; i++)
            this->sizes()[i] = sz[i];
    }

    explicit poly_vector(std::initializer_list<size_t> sz) : poly_vector(std::span{sz.begin(), sz.size()}) {}

    template<size_t I, class S> requires(I < N)
    [[nodiscard]] auto data(this S& s) noexcept {

        // TEMP make constexpr, use std::start_lifetime_as_array
        return s.base_t::template data<I + 1, cbase_t<S>>();
    }

    template<size_t I, class S> requires(I < N)
    [[nodiscard]] auto get(this S& s) noexcept {
        return std::span{s.template data<I>(), s.template size<I>()};
    }

    template<size_t I> requires(I < N)
    [[nodiscard]] size_t size(this auto const& s) noexcept {
        return s.sizes()[I];
    }

private:
    static constexpr auto make_sizes_array(std::span<size_t const> sz) {
        // Build the array of sizes for raw_poly_vector<size_t, Ts...>:
        // first entry is N (count for size_t block), then sizes for each Ts...
        std::array<size_t, N + 1> arr{};
        arr[0] = N;
        std::ranges::copy(sz, arr.begin() + 1);
        return arr;
    }
};

} // namespace cth::dt

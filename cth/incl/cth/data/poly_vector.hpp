#pragma once
#include "cth/meta/concepts.hpp"
#include "cth/meta/traits.hpp"

#include "cth/io/log.hpp"
#include "cth/ptr/move_ptr.hpp"

#include <array>
#include <span>
#include <vector>


namespace cth::dt {

template<mta::trivial... Ts>
requires(sizeof...(Ts) >= 1)
class raw_poly_vector {
public:
    static constexpr size_t MIN_ALIGN = alignof(std::max_align_t);
    static constexpr size_t SIZE = sizeof...(Ts);
    static constexpr size_t N = sizeof...(Ts);

    template<size_t I>
    requires(I < N)
    using value_type = mta::get_t<I, Ts...>;

private:
    static constexpr std::array<size_t, N> TYPE_ALIGNS{std::max(MIN_ALIGN, alignof(Ts))...};
    static constexpr auto ALLOC_ALIGN = std::align_val_t{std::ranges::max(TYPE_ALIGNS)};

    using base_t = std::byte;

public:
    explicit constexpr raw_poly_vector(std::span<size_t const> sizes) { realloc(sizes); }

    explicit constexpr raw_poly_vector(std::initializer_list<size_t> sizes) :
        raw_poly_vector(std::span{sizes}) {}

    constexpr virtual ~raw_poly_vector() { free(); }

    template<size_t I, class S>
    [[nodiscard]] constexpr auto data(this S& s) noexcept {
        static_assert(I < N, "I is out of bounds");

        using ptr_t = mta::fwd_const_t<S, value_type<I>>*;

        // TEMP use std::start_lifetime_as_array
        return std::assume_aligned<TYPE_ALIGNS[I]>(reinterpret_cast<ptr_t>(s._begins[I]));
    }

    constexpr void realloc(std::span<size_t const> sizes) {
        CTH_CRITICAL(
            sizes.size() != N,
            "invalid sizes count ({}), requires sizeof...(Ts) = {}",
            sizes.size(),
            N
        ) {}


        constexpr std::array<size_t, N> typeSizes{sizeof(Ts)...};
        constexpr std::array<size_t, N> typeAligns{std::max(MIN_ALIGN, alignof(Ts))...};
        std::array<size_t, N> byteOffsets{};

        size_t offset = 0;

        for(size_t i = 0; i < N; ++i) {
            offset = (offset + typeAligns[i] - 1) & ~(typeAligns[i] - 1);

            byteOffsets[i] = offset;

            offset += typeSizes[i] * sizes[i];
        }

        if(offset == 0)
            return;

        data() = static_cast<std::byte*>(::operator new(offset, ALLOC_ALIGN));

        for(size_t i = 0; i < N; ++i)
            _begins[i] = data() + byteOffsets[i];
    }

    void swap(this raw_poly_vector& s, raw_poly_vector& other) { std::swap(s._begins, other._begins); }

private:
    constexpr void free() {
        if(data() == nullptr)
            return;

        ::operator delete(data(), ALLOC_ALIGN);
    }

    std::array<base_t*, N> _begins{};

    base_t*& data() { return _begins[0]; }

public:
    constexpr raw_poly_vector(raw_poly_vector const& other) = delete;
    constexpr raw_poly_vector& operator=(raw_poly_vector const& other) = delete;
    constexpr raw_poly_vector(raw_poly_vector&& other) noexcept { std::swap(*this, other); }
    constexpr raw_poly_vector& operator=(raw_poly_vector&& other) noexcept {
        free();
        _begins = other._begins;
        other.data() = nullptr;
        return *this;
    };
};
}


namespace cth::dt {

template<mta::trivial... Ts>
class poly_vector {
public:
    static constexpr size_t SIZE = sizeof...(Ts);
    static constexpr size_t N = sizeof...(Ts);

private:
    using base_t = raw_poly_vector<size_t, Ts...>;

    template<class S>
    using cbase_t = mta::fwd_const_t<S, base_t>;

    template<size_t I, class S>
    requires(I <= N)
    [[nodiscard]] auto* raw_data(this S& s) noexcept {
        return s._base.template data<I>();
    }

    template<class S>
    [[nodiscard]] auto* raw_sizes(this S& s) noexcept {
        // sizes are stored in the first block (index 0) of the raw buffer
        return s.template raw_data<0>();
    }

public:
    template<size_t I>
    requires(I < N)
    using value_type = mta::get_t<I, Ts...>;

    explicit constexpr poly_vector(std::span<size_t const> sz) : _base{make_sizes_array(sz)} {
        CTH_CRITICAL(sz.size() != N, "invalid size range given") {}
        for(size_t i = 0; i < N; i++)
            this->raw_sizes()[i] = sz[i];
    }

    explicit constexpr poly_vector(std::initializer_list<size_t> sz) : poly_vector(std::span{sz}) {}

    constexpr ~poly_vector() = default;

    template<size_t I, class S>
    requires(I < N)
    [[nodiscard]] constexpr auto data(this S& s) noexcept {
        return s.template raw_data<I + 1>();
    }

    template<size_t I, class S>
    requires(I < N)
    [[nodiscard]] constexpr auto get(this S& s) noexcept {
        return std::span{s.template data<I>(), s.template size<I>()};
    }

    template<size_t I>
    requires(I < N)
    [[nodiscard]] constexpr size_t size(this auto const& s) noexcept {
        return s.raw_sizes()[I];
    }

    [[nodiscard]] constexpr std::span<size_t const> sizes(this auto const& s) noexcept {
        return {s.raw_sizes(), N};
    }

private:
    static constexpr auto make_sizes_array(std::span<size_t const> sz) {
        std::array<size_t, N + 1> arr{};
        arr[0] = N;
        std::ranges::copy(sz, arr.begin() + 1);
        return arr;
    }

    [[nodiscard]] constexpr size_t min_raw_size(this poly_vector const& s) {
        auto begin = std::bit_cast<uintptr_t>(s.template raw_data<0>());

        auto end = std::bit_cast<uintptr_t>(s.template data<N - 1>() + s.sizes()[N - 1]);

        return static_cast<size_t>(end - begin);
    }

    constexpr void copy_base_data(this poly_vector& s, poly_vector const& other) {
        auto* dst = s.template raw_data<0>();
        auto* src = other.template raw_data<0>();
        auto size = other.min_raw_size();


        std::memcpy(dst, src, size);
    }

    raw_poly_vector<size_t, Ts...> _base;

public:
    constexpr poly_vector(poly_vector const& other) : _base{make_sizes_array(other.sizes())} {
        this->copy_base_data(other);
    }
    constexpr poly_vector& operator=(poly_vector const& other) {
        auto& self = *this;

        if(&other == this)
            return self;

        self._base.realloc(make_sizes_array(other.sizes()));
        self.copy_base_data(other);

        return self;
    }
    constexpr poly_vector(poly_vector&& other) noexcept = default;
    constexpr poly_vector& operator=(poly_vector&& other) noexcept = default;
};

} // namespace cth::dt

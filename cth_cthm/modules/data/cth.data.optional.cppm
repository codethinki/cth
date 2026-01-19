module;

#include <functional>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

export module cth.data.optional;

import cth.meta.concepts;
import cth.meta.traits;

export namespace cth::dt {

/**
 * drop in std::optional replacement with support for references and void
 * @tparam T contained type
 */
template<class T>
struct optional;

struct optional_base {

    [[nodiscard]] constexpr explicit operator bool(this auto const& self) noexcept { return self.has_value(); }

    template<class Self, class F>
    constexpr auto transform(this Self&& self, F&& f) {
        using internal_t = decltype(*std::forward<Self>(self));
        constexpr bool inVoid = mta::is_void<mta::rcvr_t<internal_t>>;

        using raw_result_t = mta::conditional_pack_t<
            inVoid,
            mta::trait_pack<std::invoke_result_t, F>,
            mta::trait_pack<std::invoke_result_t, F, internal_t>
        >;

        using R = mta::rcvr_t<raw_result_t>;

        if(!self.has_value()) return optional<R>{std::nullopt};

        if constexpr(mta::is_void<R>) {
            if constexpr(inVoid)
                std::invoke(std::forward<F>(f));
            else
                std::invoke(std::forward<F>(f), *std::forward<Self>(self));

            return optional<R>{std::in_place};
        } else {
            if constexpr(inVoid)
                return optional<R>{std::invoke(std::forward<F>(f))};
            else
                return optional<R>{std::invoke(std::forward<F>(f), *std::forward<Self>(self))};
        }
    }

    template<class Self, class F>
    constexpr auto and_then(this Self&& self, F&& f) {
        using internal_t = decltype(*std::forward<Self>(self));
        constexpr bool inVoid = mta::is_void<mta::rcvr_t<internal_t>>;

        if(self.has_value()) {
            if constexpr(inVoid)
                return std::invoke(std::forward<F>(f));
            else
                return std::invoke(std::forward<F>(f), *std::forward<Self>(self));
        }

        using result_t = mta::rcvr_t<
            mta::conditional_pack_t<
                inVoid,
                mta::trait_pack<std::invoke_result_t, F>,
                mta::trait_pack<std::invoke_result_t, F, internal_t>
            >
        >;
        return result_t{std::nullopt};
    }

    template<class Self, class F>
    constexpr auto or_else(this Self&& self, F&& f) {
        if(self.has_value()) return std::forward<Self>(self);
        return std::invoke(std::forward<F>(f));
    }
};

template<class T>
struct optional : optional_base {
    constexpr optional() : optional{std::nullopt} {}
    constexpr optional(std::nullopt_t) noexcept : _value{std::nullopt} {}

    template<class U = T> requires std::constructible_from<T, U> && (!std::same_as<std::decay_t<U>, optional>)
    constexpr optional(U&& val) : _value{std::forward<U>(val)} {}

    template<class... Args>
    constexpr explicit optional(std::in_place_t, Args&&... args) : _value{std::in_place, std::forward<Args>(args)...} {}

    constexpr void swap(optional& other) noexcept { _value.swap(other._value); }
    constexpr void reset() noexcept { _value.reset(); }

    template<class... Args>
    constexpr T& emplace(Args&&... args) { return _value.emplace(std::forward<Args>(args)...); }

    [[nodiscard]] constexpr bool has_value() const noexcept { return _value.has_value(); }

    template<class Self> constexpr auto&& operator*(this Self&& self) { return *std::forward<Self>(self)._value; }
    template<class Self> constexpr auto&& operator->(this Self&& self) {
        return std::forward<Self>(self)._value.operator->();
    }

    template<class Self> constexpr auto&& value(this Self&& self) {
        if(!self.has_value()) throw std::bad_optional_access{};
        return *std::forward<Self>(self)._value;
    }

    template<class U>
    constexpr T value_or(U&& def) const & { return _value.value_or(std::forward<U>(def)); }
    template<class U>
    constexpr T value_or(U&& def) && { return std::move(_value).value_or(std::forward<U>(def)); }

private:
    std::optional<T> _value;
};

template<mta::reference T>
struct optional<T> : optional_base {
    using base_type = std::remove_reference_t<T>;

    constexpr optional() noexcept : optional{std::nullopt} {}
    constexpr optional(std::nullopt_t) noexcept : _value{nullptr} {}

    constexpr optional(T val) noexcept : _value{std::addressof(val)} {}

    constexpr void emplace(T val) noexcept { this->operator=(val); }

    constexpr void swap(optional& other) noexcept { std::swap(_value, other._value); }

    constexpr void reset() noexcept { _value = nullptr; }

    constexpr optional& operator=(std::nullopt_t) noexcept {
        reset();
        return *this;
    }

    constexpr optional& operator=(T val) noexcept {
        _value = std::addressof(val);
        return *this;
    }

    [[nodiscard]] constexpr bool has_value() const noexcept { return _value != nullptr; }

    constexpr T operator*() const { return *_value; }
    constexpr base_type* operator->() const { return _value; }

    constexpr T value() const {
        if(!_value) throw std::bad_optional_access{};
        return *_value;
    }

    template<std::convertible_to<T> U>
    constexpr T value_or(U&& def) const {
        if(_value) return *_value;
        return static_cast<T>(std::forward<U>(def));
    }

private:
    base_type* _value;
};

template<>
struct optional<void> : optional_base {
    constexpr optional() noexcept : optional{std::nullopt} {}
    constexpr optional(std::nullopt_t) noexcept : _hasValue{false} {}
    constexpr explicit optional(std::in_place_t) noexcept : _hasValue{true} {}

    constexpr void swap(optional& other) noexcept { std::swap(_hasValue, other._hasValue); }
    constexpr void reset() noexcept { _hasValue = false; }
    constexpr void emplace() noexcept { _hasValue = true; }

    [[nodiscard]] constexpr bool has_value() const noexcept { return _hasValue; }

    constexpr void operator*() const noexcept {}
    constexpr void value() const { if(!_hasValue) throw std::bad_optional_access{}; }
    constexpr void value_or() const noexcept {}

private:
    bool _hasValue;
};

}

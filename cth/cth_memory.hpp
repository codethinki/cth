#pragma once
#include <memory>
#include <type_traits>


namespace cth::memory {
/**
 * \brief move aware pointer (nullptr after move)
 * \note NOT memory safe
 */
template<typename T>
struct basic_ptr {
    using pointer = T*;
    using element_type = T;
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr basic_ptr(T* ptr) : _ptr(ptr) {}
    constexpr ~basic_ptr() = default;

private:
    T* _ptr;

public:
    constexpr void swap(basic_ptr& other) noexcept { std::swap(_ptr, other._ptr); }

    [[nodiscard]] constexpr T* get() const { return _ptr; }

    [[nodiscard]] constexpr T* release() noexcept {
        auto ptr = _ptr;
        _ptr = nullptr;
        return ptr;
    }
    [[nodiscard]] constexpr T* operator->() const { return _ptr; }
    [[nodiscard]] constexpr T& operator*() const { return *_ptr; }
    [[nodiscard]] explicit constexpr operator bool() const { return static_cast<bool>(_ptr); }

    basic_ptr(const basic_ptr& other) = default;
    basic_ptr& operator=(const basic_ptr& other) = default;

    basic_ptr(basic_ptr&& other) noexcept : _ptr(other.release()) {}
    basic_ptr& operator=(basic_ptr&& other) noexcept {
        _ptr = other.release();
        return *this;
    }


};
template<typename T>
constexpr void swap(basic_ptr<T>& left, basic_ptr<T>& right) noexcept { left.swap(right); }

template<typename T, typename U>
[[nodiscard]] constexpr bool operator==(const basic_ptr<T>& ptr, const basic_ptr<U>& other) { return ptr.get() == other.get(); }

template<typename T, typename U>
[[nodiscard]] constexpr bool operator!=(const basic_ptr<T>& left, const basic_ptr<U>& right) { return !(left.get() == right.get()); }

template<typename T>
[[nodiscard]] constexpr bool operator==(const basic_ptr<T>& ptr, std::nullptr_t) { return !ptr.get(); }

template<typename T>
[[nodiscard]] constexpr bool operator!=(const basic_ptr<T>& ptr, std::nullptr_t) { return ptr.get(); }

template<typename T, typename U>
[[nodiscard]] constexpr bool operator<(const basic_ptr<T>& left, const basic_ptr<U>& right) {
    using left_t = typename basic_ptr<T>::pointer;
    using right_t = typename basic_ptr<U>::pointer;
    using ptr_t = std::common_type_t<left_t, right_t>;

    return std::less<ptr_t>{}(left.get(), right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>(const basic_ptr<T>& left, const basic_ptr<U>& right) { return right < left; }

template<typename T, typename U>
[[nodiscard]] constexpr bool operator<=(const basic_ptr<T>& left, const basic_ptr<U>& right) { return !(right < left); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>=(const basic_ptr<T>& left, const basic_ptr<U>& right) { return !(left < right); }
template<typename T>
[[nodiscard]] constexpr bool operator<(const basic_ptr<T>& left, nullptr_t) {
    using ptr_t = typename basic_ptr<T>::pointer;
    return std::less<ptr_t>{}(left.get(), nullptr);
}
template<typename T>
[[nodiscard]] constexpr bool operator>(const basic_ptr<T>& left, nullptr_t) { return nullptr < left; }
template<typename T>
[[nodiscard]] constexpr bool operator<=(const basic_ptr<T>& left, nullptr_t) { return !(nullptr < left); }
template<typename T>
[[nodiscard]] constexpr bool operator>=(const basic_ptr<T>& left, nullptr_t) { return !(left < nullptr); }
template<typename T>
std::ostream& operator<<(std::ostream& os, const basic_ptr<T>& ptr) {
    os << ptr;
    return os;
}


}

namespace std {

template<typename T>
struct hash<cth::memory::basic_ptr<T>> {
    std::size_t operator()(const cth::memory::basic_ptr<T>& ptr) const noexcept {
        return std::hash<T*>{}(ptr.get());
    }
};
}

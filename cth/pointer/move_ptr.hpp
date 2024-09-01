#pragma once
#include <memory>
#include <type_traits>

namespace cth {
/**
 * \brief move aware pointer (nullptr after move)
 * \note NOT memory safe
 */
template<typename T>
struct move_ptr {
    using pointer = T*;
    using element_type = T;
    constexpr move_ptr() noexcept = default;
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr move_ptr(T* ptr) : _ptr(ptr) {}
    constexpr ~move_ptr() = default;

private:
    T* _ptr = nullptr;

public:
    constexpr void free() noexcept {
        delete _ptr;
        _ptr = nullptr;
    }

    constexpr void swap(move_ptr& other) noexcept { std::swap(_ptr, other._ptr); }

    [[nodiscard]] constexpr T* get() const { return _ptr; }

    /**
     * @brief returns current pointer and resets the object 
     */
    [[nodiscard]] constexpr T* release() noexcept {
        auto ptr = _ptr;
        _ptr = nullptr;
        return ptr;
    }
    [[nodiscard]] constexpr T* operator->() const { return _ptr; }
    [[nodiscard]] constexpr T& operator*() const { return *_ptr; }
    [[nodiscard]] explicit constexpr operator bool() const { return static_cast<bool>(_ptr); }

    move_ptr(move_ptr const& other) = default;
    move_ptr& operator=(move_ptr const& other) = default;

    move_ptr(move_ptr&& other) noexcept : _ptr(other.release()) {}
    move_ptr& operator=(move_ptr&& other) noexcept {
        _ptr = other.release();
        return *this;
    }


};
template<typename T>
constexpr void swap(move_ptr<T>& left, move_ptr<T>& right) noexcept { left.swap(right); }

template<typename T, typename... Args>
[[nodiscard]] constexpr move_ptr<T> make_basic(Args&&... args) { return move_ptr<T>(new T(std::forward<Args>(args)...)); }

template<typename T, typename U>
[[nodiscard]] constexpr bool operator==(move_ptr<T> const& ptr, move_ptr<U> const& other) { return ptr.get() == other.get(); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator!=(move_ptr<T> const& left, move_ptr<U> const& right) { return !(left.get() == right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<(move_ptr<T> const& left, move_ptr<U> const& right) { return std::less{}(left.get(), right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>(move_ptr<T> const& left, move_ptr<U> const& right) { return std::greater{}(left.get(), right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<=(move_ptr<T> const& left, move_ptr<U> const& right) { return std::less_equal{}(left.get(), right.get()); }
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>=(move_ptr<T> const& left, move_ptr<U> const& right) { return std::greater_equal{}(left.get(), right.get()); }

template<typename T>
[[nodiscard]] constexpr bool operator==(move_ptr<T> const& ptr, std::nullptr_t) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, move_ptr<T> const& ptr) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(move_ptr<T> const& ptr, std::nullptr_t) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t, move_ptr<T> const& ptr) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator<(move_ptr<T> const& left, nullptr_t) { return std::less<T*>{}(left.get(), nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator<(nullptr_t, move_ptr<T> const& right) { return std::less{}(nullptr, right.get()); }
template<typename T>
[[nodiscard]] constexpr bool operator>(move_ptr<T> const& left, nullptr_t) { return std::greater<T*>{}(left.get(), nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator>(nullptr_t, move_ptr<T> const& right) { return std::greater{}(nullptr, right.get()); }
template<typename T>
[[nodiscard]] constexpr bool operator<=(move_ptr<T> const& left, nullptr_t) { return std::less_equal<T*>{}(left.get(), nullptr); }
template<typename T>
[[nodiscard]] constexpr bool operator<=(nullptr_t, move_ptr<T> const& right) { return std::less_equal{}(nullptr, right.get()); };
template<typename T>
[[nodiscard]] constexpr bool operator>=(move_ptr<T> const& left, nullptr_t) { return std::greater_equal<T*>{}(left.get(), nullptr); };
template<typename T>
[[nodiscard]] constexpr bool operator>=(nullptr_t, move_ptr<T> const& right) { return std::greater_equal{}(nullptr, right.get()); }


template<typename T>
std::ostream& operator<<(std::ostream& os, move_ptr<T> const& ptr) {
    os << ptr;
    return os;
}

} // namespace cth


template<typename T>
struct std::hash<cth::move_ptr<T>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    std::size_t operator()(cth::move_ptr<T> const& ptr) const noexcept {
        using ptr_t = typename cth::move_ptr<T>::pointer;
        return std::hash<ptr_t>{}(ptr.get());
    }
};

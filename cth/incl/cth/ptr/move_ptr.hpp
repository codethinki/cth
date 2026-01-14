#pragma once
#include <memory>
#include <type_traits>

namespace cth {
/**
 * move aware pointer (nullptr after move)
 * @attention NOT memory safe
 */
template<typename T>
struct move_ptr {
    using pointer = T*;
    using element_type = T;
    constexpr move_ptr() noexcept = default;
    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr move_ptr(T* ptr) : _ptr{ptr} {}
    constexpr ~move_ptr() = default;

private:
    T* _ptr = nullptr;

public:
    constexpr void swap(this move_ptr& s, move_ptr& other) noexcept { std::swap(s._ptr, other._ptr); }

    [[nodiscard]] constexpr auto get(this auto& s) noexcept { return s._ptr; }

    /**
     * @brief returns current pointer and resets the object 
     */
    [[nodiscard]] constexpr T* release(this move_ptr& s) noexcept { return std::exchange(s._ptr, nullptr); }
    [[nodiscard]] constexpr auto operator->(this auto& s) noexcept { return s._ptr; }
    [[nodiscard]] constexpr auto operator*(this auto& s) noexcept { return *s._ptr; }
    [[nodiscard]] constexpr operator bool(this move_ptr const& s) { return static_cast<bool>(s._ptr); }

    constexpr move_ptr(move_ptr const& other) = default;
    constexpr move_ptr& operator=(move_ptr const& other) = default;

    constexpr move_ptr(move_ptr&& other) noexcept : _ptr{other.release()} {}
    constexpr move_ptr& operator=(move_ptr&& other) noexcept {
        auto& self = *this;
        self._ptr = other.release();
        return self;
    }
};
template<typename T>
constexpr void swap(move_ptr<T>& left, move_ptr<T>& right) noexcept { left.swap(right); }

template<typename T, typename... Args>
[[nodiscard]] constexpr move_ptr<T> make_move(Args&&... args) {
    return move_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T, typename U>
[[nodiscard]] constexpr bool operator==(move_ptr<T> const& ptr, move_ptr<U> const& other) {
    return ptr.get() == other.get();
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator!=(move_ptr<T> const& left, move_ptr<U> const& right) {
    return !(left.get() == right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<(move_ptr<T> const& left, move_ptr<U> const& right) {
    return std::less{}(left.get(), right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>(move_ptr<T> const& left, move_ptr<U> const& right) {
    return std::greater{}(left.get(), right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator<=(move_ptr<T> const& left, move_ptr<U> const& right) {
    return std::less_equal{}(left.get(), right.get());
}
template<typename T, typename U>
[[nodiscard]] constexpr bool operator>=(move_ptr<T> const& left, move_ptr<U> const& right) {
    return std::greater_equal{}(left.get(), right.get());
}

template<typename T>
[[nodiscard]] constexpr bool operator==(move_ptr<T> const& ptr, std::nullptr_t) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator==(std::nullptr_t, move_ptr<T> const& ptr) { return !ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(move_ptr<T> const& ptr, std::nullptr_t) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator!=(std::nullptr_t, move_ptr<T> const& ptr) { return ptr.get(); }
template<typename T>
[[nodiscard]] constexpr bool operator<(move_ptr<T> const& left, nullptr_t) {
    return std::less<T*>{}(left.get(), nullptr);
}
template<typename T>
[[nodiscard]] constexpr bool operator<(nullptr_t, move_ptr<T> const& right) {
    return std::less{}(nullptr, right.get());
}
template<typename T>
[[nodiscard]] constexpr bool operator>(move_ptr<T> const& left, nullptr_t) {
    return std::greater<T*>{}(left.get(), nullptr);
}
template<typename T>
[[nodiscard]] constexpr bool operator>(nullptr_t, move_ptr<T> const& right) {
    return std::greater{}(nullptr, right.get());
}
template<typename T>
[[nodiscard]] constexpr bool operator<=(move_ptr<T> const& left, nullptr_t) {
    return std::less_equal<T*>{}(left.get(), nullptr);
}
template<typename T>
[[nodiscard]] constexpr bool operator<=(nullptr_t, move_ptr<T> const& right) {
    return std::less_equal{}(nullptr, right.get());
};
template<typename T>
[[nodiscard]] constexpr bool operator>=(move_ptr<T> const& left, nullptr_t) {
    return std::greater_equal<T*>{}(left.get(), nullptr);
};
template<typename T>
[[nodiscard]] constexpr bool operator>=(nullptr_t, move_ptr<T> const& right) {
    return std::greater_equal{}(nullptr, right.get());
}


template<typename T>
constexpr std::ostream& operator<<(std::ostream& os, move_ptr<T> const& ptr) {
    os << ptr;
    return os;
}

} // namespace cth


template<typename T>
struct std::hash<cth::move_ptr<T>> /* NOLINT(cert-dcl58-cpp) this is a valid overload for std::hash */ {
    constexpr std::size_t operator()(cth::move_ptr<T> const& ptr) const noexcept {
        using ptr_t = cth::move_ptr<T>::pointer;
        return std::hash<ptr_t>{}(ptr.get());
    }
};

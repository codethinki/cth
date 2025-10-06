#pragma once
#include "cth/types/typ_variadic.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace cth::str {
class joiner {
#define CTH_DEV_JOINER_FORWARD_STRING_METHOD(str, method_name)\
    template<typename... Args>\
    [[nodiscard]] auto method_name(this auto&& self, Args&&... args)\
        requires requires (std::string s, Args... args) { s.method_name(std::forward<Args>(args)...); } {\
        return self.str.method_name(std::forward<Args>(args)...);\
    }

public:
    explicit joiner(std::string delimiter = " ")
        : _delimiter(std::move(delimiter)) {}

    joiner(std::string delimiter, size_t capacity)
        : _delimiter(std::move(delimiter)) { _buffer.reserve(capacity); }

    template<typename T>
    auto& operator+=(this auto&& self, T&& value) {
        if(!self._buffer.empty())
            self._buffer.append(self._delimiter);

        if constexpr(requires { self._buffer.append(std::forward<T>(value)); })
            self._buffer.append(std::forward<T>(value));
        else
            self._buffer.append(std::to_string(std::forward<T>(value)));

        return self;
    }

    [[nodiscard]] std::string string() const & { return _buffer; }
    [[nodiscard]] std::string string() && { return std::move(_buffer); }
    [[nodiscard]] std::string const& delim() const { return _delimiter; }
    operator std::string() const & { return _buffer; }

    operator std::string() && { return std::move(_buffer); }
    [[nodiscard]] operator std::string_view() const { return _buffer; }

    [[nodiscard]] auto begin(this auto&& self) { return self._buffer.begin(); }
    [[nodiscard]] auto end(this auto&& self) { return self._buffer.end(); }
    [[nodiscard]] auto rbegin(this auto&& self) { return self._buffer.rbegin(); }
    [[nodiscard]] auto rend(this auto&& self) { return self._buffer.rend(); }
    [[nodiscard]] auto& operator[](this auto&& self, size_t pos) { return self._buffer[pos]; }
    [[nodiscard]] auto& at(this auto&& self, size_t pos) { return self._buffer.at(pos); }
    [[nodiscard]] auto& front(this auto&& self) { return self._buffer.front(); }
    [[nodiscard]] auto& back(this auto&& self) { return self._buffer.back(); }

    [[nodiscard]] bool empty() const { return _buffer.empty(); }
    [[nodiscard]] size_t size() const { return _buffer.size(); }
    [[nodiscard]] size_t length() const { return _buffer.length(); }
    [[nodiscard]] size_t capacity() const { return _buffer.capacity(); }
    void reserve(size_t capacity) { _buffer.reserve(capacity); }
    void shrink_to_fit() { _buffer.shrink_to_fit(); }
    void clear() { _buffer.clear(); }

    [[nodiscard]] char const* c_str() const { return _buffer.c_str(); }
    [[nodiscard]] char const* data() const { return _buffer.data(); }

    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, compare)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, starts_with)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, ends_with)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, contains)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, find)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, rfind)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, find_first_of)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, find_last_of)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, find_first_not_of)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, find_last_not_of)

    template<typename... Args>
    [[nodiscard]] auto substr(this auto&& self, Args&&... args) requires requires(std::string str, Args... args) {
        std::string_view(str).substr(std::forward<Args>(args)...);
    } { return std::string_view(self._buffer).substr(std::forward<Args>(args)...); }

private:
    std::string _buffer;
    std::string _delimiter;
};

template<typename T>
[[nodiscard]] auto operator+(joiner&& sj, T&& value) {
    sj += std::forward<T>(value);
    return std::move(sj);
}

template<typename T>
[[nodiscard]] auto operator+(joiner const& sj, T&& value) {
    auto copy = sj;
    copy += std::forward<T>(value);
    return copy;
}
}

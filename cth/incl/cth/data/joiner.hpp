#pragma once
#include "cth/meta/variadic.hpp"

#include <string>
#include <string_view>
#include <utility>

namespace cth::dt {
class joiner {
#define CTH_DEV_JOINER_FORWARD_STRING_METHOD(str, method_name)                                               \
    template<class... Args>                                                                                  \
    [[nodiscard]] auto method_name(this auto&& self, Args&&... args)                                         \
        requires requires(std::string s, Args... args) { s.method_name(std::forward<Args>(args)...); }       \
    {                                                                                                        \
        return self.str.method_name(std::forward<Args>(args)...);                                            \
    }

public:
    explicit joiner(std::string delimiter = " ") : _delimiter(std::move(delimiter)) {}

    joiner(std::string delimiter, size_t capacity) : _delimiter(std::move(delimiter)) {
        _buffer.reserve(capacity);
    }

    template<class T>
    auto& operator+=(this auto&& self, T&& value) {
        if(!self._buffer.empty())
            self._buffer.append(self._delimiter);

        if constexpr(requires { self._buffer.append(std::forward<T>(value)); })
            self._buffer.append(std::forward<T>(value));
        else
            self._buffer.append(std::to_string(std::forward<T>(value)));

        return self;
    }

    [[nodiscard]] std::string const& string() const& { return _buffer; }
    [[nodiscard]] std::string string() && { return std::move(_buffer); }
    [[nodiscard]] std::string const& delim() const { return _delimiter; }

    operator std::string() const& { return _buffer; }
    operator std::string() && { return std::move(_buffer); }
    [[nodiscard]] operator std::string_view() const { return _buffer; }

    bool operator==(joiner const&) const = default;

    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, begin)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, end)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, rbegin)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, rend)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, operator[])
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, at)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, front)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, back)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, empty)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, size)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, length)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, capacity)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, reserve)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, shrink_to_fit)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, clear)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, c_str)
    CTH_DEV_JOINER_FORWARD_STRING_METHOD(_buffer, data)
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

    template<class... Args>
    [[nodiscard]] auto substr(this auto&& self, Args&&... args)
        requires requires(std::string str, Args... args) {
            std::string_view{str}.substr(std::forward<Args>(args)...);
        }
    {
        return std::string_view{self._buffer}.substr(std::forward<Args>(args)...);
    }

private:
    std::string _buffer;
    std::string _delimiter;
};

template<class T>
[[nodiscard]] auto operator+(joiner&& sj, T&& value) {
    sj += std::forward<T>(value);
    return std::move(sj);
}

template<class T>
[[nodiscard]] auto operator+(joiner const& sj, T&& value) {
    auto copy = sj;
    copy += std::forward<T>(value);
    return copy;
}

[[nodiscard]] inline bool operator==(joiner const& sj, std::string const& str) { return sj.string() == str; }
[[nodiscard]] inline bool operator==(std::string const& str, joiner const& sj) { return sj == str; }
[[nodiscard]] inline bool operator==(joiner const& sj, std::string_view str) {
    return std::string_view{sj} == str;
}
[[nodiscard]] inline bool operator==(std::string_view str, joiner const& sj) { return sj == str; }


}

#pragma once
#include <cth/exception.hpp>
#include <cth/io/log.hpp>

#include <system_error>

namespace cth::except {
/**
 * exception of cth_coro
 * @details
 * - prefixes the message with `"cth_coro: "`
 * - `code()`: the error code of a failed wait, empty -> no wait error
 */
class coro_exception : public default_exception {
public:
    coro_exception(
        std::string_view msg,
        Severity severity = Severity::ERR,
        std::source_location const& loc = {},
        std::stacktrace trace = {}
    ) : default_exception{std::format("cth_coro: {}", msg), severity, loc, std::move(trace)} {}

    using default_exception::add;

    /**
     * stores the error code and adds it to the message
     */
    coro_exception& add(std::error_code code) {
        _code = code;
        return default_exception::add("error code: ({}) {}", code.category().name(), code.value());
    }

    /**
     * @return error code of the failed wait, empty -> no wait error
     */
    [[nodiscard]] std::error_code const& code() const noexcept { return _code; }

private:
    std::error_code _code{};
};
}


#define CTH_CORO_WARN(expression, fmt_message, ...)                                                          \
    CTH_WARN_T(cth::except::coro_exception, expression, fmt_message, __VA_ARGS__)

#define CTH_CORO_STABLE_ERR(expression, fmt_message, ...)                                                    \
    CTH_STABLE_ERR_T(cth::except::coro_exception, expression, fmt_message, __VA_ARGS__)

#define CTH_CORO_STABLE_THROW(expression, fmt_message, ...)                                                  \
    CTH_STABLE_THROW_T(cth::except::coro_exception, expression, fmt_message, __VA_ARGS__)

/**
 * acts like `CTH_CORO_STABLE_THROW` and adds the error code to the exception
 * @param expression `true` -> throws
 * @param code to add to the exception, see `cth::except::coro_exception::code()`
 * @param fmt_message format string
 * @details braces and else work like the other CTH_ log macros
 */
#define CTH_CORO_CODE_STABLE_THROW(expression, code, fmt_message, ...)                                      \
    CTH_CORO_STABLE_THROW(expression, fmt_message, __VA_ARGS__)                                              \
        for(auto&& _ : {(details->e().add(code), 0)})

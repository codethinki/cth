#pragma once
#include <cth/exception.hpp>
#include <cth/io/log.hpp>

namespace cth::except {
class coro_exception : public default_exception {
public:
    coro_exception(
        std::string_view msg,
        Severity severity = Severity::ERR,
        std::source_location const& loc = {},
        std::stacktrace trace = {}
    ) : default_exception{std::format("cth_coro: {}", msg), severity, loc, std::move(trace)} {}
};
}


#define CTH_CORO_WARN(expression, fmt_message, ...)                                                          \
    CTH_WARN_T(cth::except::coro_exception, expression, fmt_message, __VA_ARGS__)

#define CTH_CORO_STABLE_ERR(expression, fmt_message, ...)                                                    \
    CTH_STABLE_ERR_T(cth::except::coro_exception, expression, fmt_message, __VA_ARGS__)

#define CTH_CORO_STABLE_THROW(expression, fmt_message, ...)                                                  \
    CTH_STABLE_THROW_T(cth::except::coro_exception, expression, fmt_message, __VA_ARGS__)

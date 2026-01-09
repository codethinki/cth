#pragma once
#include <cth/exception.hpp>

namespace cth::except {
class coro_exception : public default_exception {
public:
    coro_exception(
        std::string_view msg,
        Severity severity = Severity::ERR,
        std::source_location const& loc = {},
        std::stacktrace trace = {}
    ) : default_exception{
        std::format("cth_coro: {}", msg),
        severity,
        loc,
        std::move(trace)
    } {}
};
}

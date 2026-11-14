#pragma once
#include <cth/exception.hpp>
#include <cth/io/log.hpp>

#include <cstddef>
#include <exception>
#include <format>
#include <source_location>
#include <stacktrace>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

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

/**
 * exception of `co::all()`: children of it failed
 * @details
 * - `exceptions()`: one per child in child order, `nullptr` -> the child ended without an exception
 * - the message counts the failed children, a detail line per failed child gives its `what()`
 */
class group_coro_exception : public coro_exception {
public:
    /**
     * @param exceptions one per child in child order, `nullptr` -> the child ended without an exception
     * @pre at least one exception is not `nullptr`
     */
    explicit group_coro_exception(
        std::vector<std::exception_ptr> exceptions,
        std::source_location const& loc = std::source_location::current(),
        std::stacktrace trace = std::stacktrace::current()
    ) :
        coro_exception{message(exceptions), Severity::ERR, loc, std::move(trace)},
        _exceptions{std::move(exceptions)} {
        for(size_t i = 0; i < _exceptions.size(); ++i)
            if(_exceptions[i] != nullptr) add("child {}: {}", i, what_of(_exceptions[i]));
    }

    /**
     * @return one per child in child order, `nullptr` -> the child ended without an exception
     */
    [[nodiscard]] std::vector<std::exception_ptr> const& exceptions() const noexcept { return _exceptions; }

private:
    [[nodiscard]] static std::string message(std::vector<std::exception_ptr> const& exceptions) {
        size_t failed = 0;
        for(auto const& exception : exceptions)
            if(exception != nullptr) ++failed;
        return std::format("{} of {} children of co::all() failed", failed, exceptions.size());
    }

    /**
     * @return `what()` of a `std::exception`, a fixed text for other exceptions
     */
    [[nodiscard]] static std::string what_of(std::exception_ptr const& exception) {
        try {
            std::rethrow_exception(exception);
        }
        catch(std::exception const& e) {
            return e.what();
        }
        catch(...) {
            return "an exception that does not derive std::exception";
        }
    }

    std::vector<std::exception_ptr> _exceptions;
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

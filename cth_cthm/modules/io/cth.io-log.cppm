module;
#include "cthm/macro/utility.hpp"
#include "cthm/macro/log_details/log_level.hpp"

#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

export module cth.io:log;

import cth.exception;

import :console;


namespace cth::log::dev {
inline bool colored = true;
inline io::col_stream logStream{&std::cerr, io::error.state()};

cxpr io::Text_Colors text_color(cth::except::Severity severity) {
    switch(severity) {
        case cth::except::LOG: return io::WHITE_TEXT_COL;
        case cth::except::Severity::INFO: return io::DARK_CYAN_TEXT_COL;
        case cth::except::Severity::WARNING: return io::DARK_YELLOW_TEXT_COL;
        case cth::except::Severity::ERR: return io::DARK_RED_TEXT_COL;
        case cth::except::Severity::CRITICAL: return io::DARK_RED_TEXT_COL;
        case except::SEVERITY_SIZE:
        default: std::unreachable();
    }
}

[[nodiscard]] cxpr std::string_view label(cth::except::Severity severity) {
    switch(severity) {
        case cth::except::Severity::LOG: return "[LOG]";
        case cth::except::Severity::INFO: return "[INFO]";
        case cth::except::Severity::WARNING: return "[WARNING]";
        case cth::except::Severity::ERR: return "[ERROR]";
        case cth::except::Severity::CRITICAL: return "[CRITICAL]";
        case cth::except::Severity::SEVERITY_SIZE:
        default: std::unreachable();
    }
}
}


export namespace cth::log {
/**
 * \brief sets a colored log stream. nullptr -> colored console output
 */
inline void set_log_stream(io::col_stream const& stream) {
    dev::colored = true;
    dev::logStream = stream;
}
inline void set_log_stream(std::ostream* stream) {
    dev::colored = false;
    dev::logStream = io::col_stream{stream};
}


inline void msg(cth::except::Severity severity, std::string_view message) {
    if(severity < CTH_LOG_LEVEL) return;

    if(dev::colored) {
        dev::logStream.pushState();
        dev::logStream.setTextStyle(io::ITALIC_TEXT_STYLE);
        dev::logStream.setTextStyle(io::UNDERLINED_TEXT_STYLE);
        dev::logStream.print(dev::text_color(severity), dev::label(severity));
        dev::logStream.popState();
        dev::logStream.print(" ");
        dev::logStream.pushState();
        dev::logStream.setTextStyle(io::BOLD_TEXT_STYLE);
        dev::logStream.println(dev::text_color(severity), message);
        dev::logStream.popState();
    } else {
        dev::logStream.print(dev::label(severity));
        dev::logStream.print(" ");
        dev::logStream.println(message);
    }
}
template<cth::except::Severity S = cth::except::LOG>
void msg(std::string_view message) noexcept {
    if constexpr(S < CTH_LOG_LEVEL) return;

    cth::log::msg(S, message);
}
template<class... Args> requires (sizeof...(Args) > 0u)
void msg(cth::except::Severity severity, std::format_string<Args...> f_str, Args&&... args) noexcept {
    if(severity < CTH_LOG_LEVEL) return;
    log::msg(severity, std::format(f_str, std::forward<Args>(args)...));
}

template<cth::except::Severity S = cth::except::LOG, class... Args> requires(sizeof...(Args) > 0u)
void msg(std::format_string<Args...> f_str, Args&&... args) noexcept {
    if constexpr(S < CTH_LOG_LEVEL) return;
    log::msg<S>(std::format(f_str, std::forward<Args>(args)...));
}



namespace dev {

    /**
     * \brief executes the statement in its destructor to support code execution before aborting / throwing
     */
    template<cth::except::Severity S, std::derived_from<except::default_exception> E = except::default_exception>
    struct LogObj {
        explicit LogObj(E exception) : _exception{std::move(exception)} {}
        ~LogObj() {
            if(_moved) return;

            print();

            if constexpr(S == cth::except::Severity::CRITICAL) std::terminate();
        }
        void add(std::string_view message) noexcept { _exception.add(std::string{message}); }
        template<class... Types> requires (sizeof...(Types) > 0u)
        void add(std::format_string<Types...> f_str, Types&&... types) {
            _exception.add(f_str, std::forward<Types>(types)...);
        }

        [[nodiscard]] E& e(this auto& self) { return self._exception; }

    private:
        E _exception;
        bool _moved = false;

    public:
        [[nodiscard]] std::string string() const { return _exception.string(); }
        [[nodiscard]] E exception() const { return _exception; }

        void print() const {
            if constexpr(static_cast<int>(S) < CTH_LOG_LEVEL)
                return;

            std::string out = "\n";

            switch(S) {
                case except::CRITICAL: out += _exception.string();
                    break;
                case except::ERR: out += _exception.string();
                    break;
                case except::WARNING: out += std::format(
                        "{0} {1} {2} {3}",
                        _exception.msg(),
                        _exception.details(),
                        _exception.func_string(),
                        _exception.loc_string()
                    );
                    break;
                case except::INFO: out += std::format(
                        "{0} {1} {2}",
                        _exception.msg(),
                        _exception.details(),
                        _exception.func_string()
                    );
                    break;
                case except::LOG: out += std::format("{0} {1}", _exception.msg(), _exception.details());
                    break;
                case except::SEVERITY_SIZE: std::unreachable();
                default: std::unreachable();
            }
            cth::log::msg(S, out);
        }

        void throwE() {
            _moved = true;
            print();
            throw _exception;
        }

        LogObj(LogObj const& other) = default;
        LogObj(LogObj&& other) noexcept : _exception{std::move(other._exception)} { other._moved = true; }
        LogObj& operator=(LogObj const& other) = default;
        LogObj& operator=(LogObj&& other) noexcept {
            other._moved = true;
            _exception = std::move(other._exception);
            return *this;
        }
    };
}

}


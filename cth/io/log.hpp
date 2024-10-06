#pragma once

// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
// disabled because this header is forced to use macros
#include "console.hpp"
#include "../constants.hpp"
#include "../exception.hpp"



#define CTH_LOG_LEVEL_ALL 0
#define CTH_LOG_LEVEL_DEBUG 0
#define CTH_LOG_LEVEL_INFO 1
#define CTH_LOG_LEVEL_WARN 2
#define CTH_LOG_LEVEL_ERR 3
#define CTH_LOG_LEVEL_CRITICAL 4
#define CTH_LOG_LEVEL_NONE 5

#ifndef CTH_LOG_LEVEL
#define CTH_LOG_LEVEL CTH_LOG_LEVEL_ALL
#endif

#include <string>
#include <utility>


namespace cth::log::dev {
inline bool colored = true;
inline io::col_stream logStream{&std::cerr, io::error.state()}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static constexpr io::Text_Colors textColor(cth::except::Severity severity) {
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

constexpr static std::string_view label(cth::except::Severity severity) {
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


namespace cth::log {
/**
 * \brief sets a colored log stream. nullptr -> colored console output
 */
inline void setLogStream(io::col_stream const& stream) {
    dev::colored = true;
    dev::logStream = stream;
}
inline void setLogStream(std::ostream* stream) {
    dev::colored = false;
    dev::logStream = io::col_stream{stream};
}


inline void msg(cth::except::Severity severity, std::string_view message) {
    if(severity < CTH_LOG_LEVEL) return;

    if(dev::colored) {
        dev::logStream.pushState();
        dev::logStream.setTextStyle(io::ITALIC_TEXT_STYLE);
        dev::logStream.setTextStyle(io::UNDERLINED_TEXT_STYLE);
        dev::logStream.print(dev::textColor(severity), dev::label(severity));
        dev::logStream.popState();
        dev::logStream.print(" ");
        dev::logStream.pushState();
        dev::logStream.setTextStyle(io::BOLD_TEXT_STYLE);
        dev::logStream.println(dev::textColor(severity), message);
        dev::logStream.popState();
    } else {
        dev::logStream.print(dev::label(severity));
        dev::logStream.println(message);
    }
}
template<cth::except::Severity S = cth::except::LOG>
void msg(std::string_view message) noexcept {
    if constexpr(S < CTH_LOG_LEVEL) return;

    cth::log::msg(S, message);
}
template<typename... Args> requires (sizeof...(Args) > 0u)
void msg(cth::except::Severity severity, std::format_string<Args...> f_str, Args&&... args) noexcept {
    if(severity < CTH_LOG_LEVEL) return;
    log::msg(severity, std::format(f_str, std::forward<Args>(args)...));
}

template<cth::except::Severity S = cth::except::LOG, typename... Args> requires(sizeof...(Args) > 0u)
void msg(std::format_string<Args...> f_str, Args&&... args) noexcept {
    if constexpr(S < CTH_LOG_LEVEL) return;
    log::msg<S>(std::format(f_str, std::forward<Args>(args)...));
}



namespace dev {
    /**
     * \brief executes the statement in its destructor to support code execution before aborting
     */
    template<cth::except::Severity S>
    struct LogObj {
        explicit LogObj(cth::except::default_exception exception) : _exception{std::move(exception)} {}
        ~LogObj() {
            if constexpr(static_cast<int>(S) < CTH_LOG_LEVEL) return;
            if(_moved) return;

            std::string out = "\n";

            switch(S) {
                case except::CRITICAL: out += _exception.string();
                    break;
                case except::ERR: out += _exception.string();
                    break;
                case except::WARNING: out += std::format("{0} {1} {2} {3}", _exception.msg(), _exception.details(), _exception.func_string(),
                        _exception.loc_string());
                    break;
                case except::INFO: out += std::format("{0} {1} {2}", _exception.msg(), _exception.details(), _exception.func_string());
                    break;
                case except::LOG: out += std::format("{0} {1}", _exception.msg(), _exception.details());
                    break;
                case except::SEVERITY_SIZE: std::unreachable();
                default: std::unreachable();
            }
            cth::log::msg(S, out);

            if constexpr(S == cth::except::Severity::CRITICAL) std::terminate();
        }
        void add(std::string_view message) noexcept { _exception.add(message.data()); }
        template<typename... Types> requires (sizeof...(Types) > 0u)
        void add(std::format_string<Types...> f_str, Types&&... types) { _exception.add(f_str, std::forward<Types>(types)...); }

    private:
        cth::except::default_exception _exception;
        bool _moved = false;

    public:
        [[nodiscard]] std::string string() const { return _exception.string(); }
        [[nodiscard]] cth::except::default_exception exception() const { return _exception; }

        LogObj(LogObj const& other) = default;
        LogObj(LogObj&& other) noexcept : _exception{std::move(other._exception)} {
            other._moved = true;
        }
        LogObj& operator=(LogObj const& other) = default;
        LogObj& operator=(LogObj&& other) noexcept {
            other._moved = true;
            _exception = std::move(other._exception);
            return *this;
        }
    };


    /**
     * \brief wrapper for dev::LogObj
     * \param expression (expression) == false -> code execution + delayed log message
     * \param message log message
     * \param severity log severity
     */
#define CTH_DEV_DELAYED_LOG_TEMPLATE(severity, expr, message_str, ...) \
     if(auto const details = static_cast<bool>(expr) ?\
            std::make_unique<log::dev::LogObj<severity>>(\
                cth::except::default_exception{std::format(message_str, __VA_ARGS__),\
                severity, std::source_location::current(), std::stacktrace::current()}\
            ) :\
            nullptr;\
        static_cast<bool>(expr)) [[unlikely]]

#define CTH_DEV_DISABLED_CRITICAL_TEMPLATE(expr) \
    /*TEMP currently not supported by msvc[[assume(!static_cast<bool>(expr))]]; \*/\
    __assume(!static_cast<bool>(expr));\
    if(std::unique_ptr<log::dev::LogObj<cth::except::Severity::CRITICAL>> details = nullptr; static_cast<bool>(expr)) [[unlikely]]

#define CTH_DEV_DISABLED_LOG_TEMPLATE() if(std::unique_ptr<cth::log::dev::LogObj<cth::except::Severity::LOG>> details = nullptr; false) //{...}

} // namespace dev


//------------------------------
//  CTH_STABLE_LOGS
//------------------------------
/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == false -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ASSERT(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::CRITICAL, !(expression), message, __VA_ARGS__)

/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ABORT(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::CRITICAL, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before error-msg (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> error-msg
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ERR(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::ERR, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before warn (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> warn
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_WARN(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::WARNING, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before info (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> info
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_INFORM(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::INFO, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before log (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> log
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_LOG(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE( cth::except::Severity::LOG, expression, message, __VA_ARGS__)


//------------------------------
//        CTH_LOGS
//------------------------------
#define CTH_ASSERT(expression, message, ...) CTH_DEV_DISABLED_CRITICAL_TEMPLATE(!static_cast<bool>(expression))
#define CTH_CRITICAL(expression, message, ...) CTH_DEV_DISABLED_CRITICAL_TEMPLATE(expression)

#define CTH_ERR(expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE()
#define CTH_WARN(expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE()
#define CTH_INFORM(expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE()
#define CTH_LOG(expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE()

#ifdef CTH_DEBUG_MODE
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_NONE

#undef CTH_ASSERT
/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == false -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
 */
#define CTH_ASSERT(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::CRITICAL, !(expression), message, __VA_ARGS__)


#undef CTH_CRITICAL
/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note #ifndef _DEBUG -> disabled
 */
#define CTH_CRITICAL(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::CRITICAL, expression, message, __VA_ARGS__)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_CRITICAL

#undef CTH_ERR
/**
  * \brief can execute code before error-msg (use {} for multiple lines)
  * \param expression static_cast<bool>(expression) == true -> error-msg
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_ERR(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::ERR, expression, message, __VA_ARGS__)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_ERR


#undef CTH_WARN
/**
  * \brief can execute code before warning (use {} for multiple lines)
  * \param expression static_cast<bool>(expression) == true -> warning
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_WARN(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::WARNING, expression, message, __VA_ARGS__)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_WARN

#undef CTH_INFORM
/**
  * \brief can execute code before info (use {} for multiple lines)
  * \param expression static_cast<bool>(expression) == true -> inform
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_INFORM(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::INFO, expression, message, __VA_ARGS__)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_INFO

#undef CTH_LOG
/**
  * \brief can execute code before log (use {} for multiple lines)
  * \param expression static_cast<bool>(expression) == true -> log
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_LOG(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::LOG, expression, message, __VA_ARGS__)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_DEBUG
    inline auto x = []() {
        CTH_ASSERT(false, "invalid CTH_LOG_LEVEL macro value defined");
        static_assert(false);
        return 10;
        }();
#endif

#endif
#endif
#endif
#endif
#endif

#endif
} // namespace cth::log

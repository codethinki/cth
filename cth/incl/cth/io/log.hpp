#pragma once

// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
// disabled because this header is forced to use macros
#include "console.hpp"
#include "cth/constants.hpp"
#include "cth/exception.hpp"


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

static cxpr io::Text_Colors text_color(cth::except::Severity severity) {
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

[[nodiscard]] cxpr static std::string_view label(cth::except::Severity severity) {
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
        template<typename... Types> requires (sizeof...(Types) > 0u)
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


    /**
     * \brief wrapper for dev::LogObj
     * \param expression (expression) == false -> code execution + delayed log message
     * \param fmt_message log message
     * \param severity log severity
     */
#define CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, severity, expression, fmt_message, ...) \
    if (auto const details = static_cast<bool>(expression) \
                ? std::make_unique<cth::log::dev::LogObj<severity, type>>(\
                    type{\
                      std::format(fmt_message, __VA_ARGS__),\
                      severity,\
                      std::source_location::current(),\
                      std::stacktrace::current()\
                    }\
                  )\
                : std::unique_ptr<cth::log::dev::LogObj<severity, type>>{nullptr};\
        static_cast<bool>(expression)\
    ) [[unlikely]]

#define CTH_DEV_DELAYED_LOG_TEMPLATE(severity, expr, fmt_message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(cth::except::default_exception, severity, expr, fmt_message, __VA_ARGS__)


#define CTH_DEV_DISABLED_CRITICAL_TEMPLATE_T(type, expr) \
    [[assume(!static_cast<bool>(expr))]]\
    if (std::unique_ptr<cth::log::dev::LogObj<cth::except::Severity::CRITICAL, type>> details = nullptr; \
        static_cast<bool>(expr))\
            for(std::unreachable();;)

#define CTH_DEV_DISABLED_CRITICAL_TEMPLATE(expr) \
    CTH_DEV_DISABLED_CRITICAL_TEMPLATE_T(cth::except::default_exception, expr)

#define CTH_DEV_DISABLED_LOG_TEMPLATE_T(type, severity) \
    if (std::unique_ptr<cth::log::dev::LogObj<severity, type>> details = nullptr; false)

#define CTH_DEV_DISABLED_LOG_TEMPLATE() CTH_DEV_DISABLED_LOG_TEMPLATE_T(cth::except::default_exception)


#define CTH_DEV_LOG_AUTO_THROW_TEMPLATE_T(type, severity, expression, message, ...)                                        \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, severity, expression, message, __VA_ARGS__)                                       \
        for(; details != nullptr; details->throwE())
}


//------------------------------
//  CTH_STABLE_LOGS
//------------------------------
/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == false -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ASSERT_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::CRITICAL, !(expression), message, __VA_ARGS__)

/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == false -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ASSERT(expression, message, ...) \
    CTH_STABLE_ASSERT_T(cth::except::default_exception, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == true -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ABORT_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::CRITICAL, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ABORT(expression, message, ...) \
    CTH_STABLE_ABORT_T(cth::except::default_exception, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before error-msg (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == true -> error-msg
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ERR_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::ERR, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before error-msg (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> error-msg
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_ERR(expression, message, ...) \
    CTH_STABLE_ERR_T(cth::except::default_exception, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before warn (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == true -> warn
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_WARN_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::WARNING, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before warn (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> warn
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_WARN(expression, message, ...) \
    CTH_STABLE_WARN_T(cth::except::default_exception, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before info (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == true -> info
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_INFO_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::INFO, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before info (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> info
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_INFO(expression, message, ...) \
    CTH_STABLE_INFO_T(cth::except::default_exception, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before log (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == true -> log
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_LOG_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::LOG, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before log (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> log
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_LOG(expression, message, ...) \
    CTH_STABLE_LOG_T(cth::except::default_exception, expression, message, __VA_ARGS__)


/**
 * \brief can execute code before throwing (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == true -> throw
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_THROW_T(type, expression, message, ...) \
    CTH_DEV_LOG_AUTO_THROW_TEMPLATE_T(type, cth::except::Severity::ERR, expression, message, __VA_ARGS__)

/**
 * \brief can execute code before throwing (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> throw
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note stable -> never disabled
 */
#define CTH_STABLE_THROW(expression, message, ...) \
    CTH_STABLE_THROW_T(cth::except::default_exception, expression, message, __VA_ARGS__)

//------------------------------
//        CTH_LOGS
//------------------------------
#define CTH_ASSERT_T(type, expression, message, ...) \
    CTH_DEV_DISABLED_CRITICAL_TEMPLATE_T(type, !static_cast<bool>(expression))
#define CTH_ASSERT(expression, message, ...) \
    CTH_ASSERT_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_CRITICAL_T(type, expression, message, ...) CTH_DEV_DISABLED_CRITICAL_TEMPLATE_T(type, expression)
#define CTH_CRITICAL(expression, message, ...) \
    CTH_CRITICAL_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_ERR_T(type, expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE_T(type, cth::except::Severity::ERR)
#define CTH_ERR(expression, message, ...) CTH_ERR_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_WARN_T(type, expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE_T(type, cth::except::Severity::WARNING)
#define CTH_WARN(expression, message, ...) CTH_WARN_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_INFO_T(type, expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE_T(type, cth::except::Severity::INFO)
#define CTH_INFO(expression, message, ...) CTH_INFO_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_LOG_T(type, expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE_T(type, cth::except::Severity::LOG)
#define CTH_LOG(expression, message, ...) CTH_LOG_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_THROW_T(type, expression, message, ...) CTH_DEV_DISABLED_LOG_TEMPLATE_T(type, cth::except::Severity::ERR)
#define CTH_THROW(expression, message, ...) CTH_THROW_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#ifdef CTH_DEBUG_MODE
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_NONE

#undef CTH_ASSERT_T
#undef CTH_ASSERT
/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == false -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
 */
#define CTH_ASSERT_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::CRITICAL, !(expression), message, __VA_ARGS__)
/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == false -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
 */
#define CTH_ASSERT(expression, message, ...) \
    CTH_ASSERT_T(cth::except::default_exception, expression, message, __VA_ARGS__)


#undef CTH_CRITICAL_T
#undef CTH_CRITICAL
/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == true -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note #ifndef _DEBUG -> disabled
 */
#define CTH_CRITICAL_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::CRITICAL, expression, message, __VA_ARGS__)
/**
 * \brief can execute code before abort (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> abort
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note #ifndef _DEBUG -> disabled
 */
#define CTH_CRITICAL(expression, message, ...) \
    CTH_CRITICAL_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_CRITICAL

#undef CTH_THROW_T
#undef CTH_THROW
/**
 * \brief can execute code before throwing (use {} for multiple lines)
 * \param type exception type
 * \param expression static_cast<bool>(expression) == true -> throw
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note #ifndef _DEBUG -> disabled
 */
#define CTH_THROW_T(type, expression, message, ...) \
    CTH_DEV_LOG_AUTO_THROW_TEMPLATE_T(type, cth::except::Severity::ERR, expression, message, __VA_ARGS__)
/**
 * \brief can execute code before throwing (use {} for multiple lines)
 * \param expression static_cast<bool>(expression) == true -> throw
 * \param message std::format_string
 * \param ... std::format arguments
 * \note this macro MUST be followed by ; or {}
 * \note #ifndef _DEBUG -> disabled
 */
#define CTH_THROW(expression, message, ...) \
    CTH_THROW_T(cth::except::default_exception, expression, message, __VA_ARGS__)


#undef CTH_ERR_T
#undef CTH_ERR
/**
  * \brief can execute code before error-msg (use {} for multiple lines)
  * \param type exception type
  * \param expression static_cast<bool>(expression) == true -> error-msg
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_ERR_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::ERR, expression, message, __VA_ARGS__)
/**
  * \brief can execute code before error-msg (use {} for multiple lines)
  * \param expression static_cast<bool>(expression) == true -> error-msg
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_ERR(expression, message, ...) CTH_ERR_T(cth::except::default_exception, expression, message, __VA_ARGS__)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_ERR


#undef CTH_WARN_T
#undef CTH_WARN
/**
  * \brief can execute code before warning (use {} for multiple lines)
  * \param type exception type
  * \param expression static_cast<bool>(expression) == true -> warning
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_WARN_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::WARNING, expression, message, __VA_ARGS__)
/**
  * \brief can execute code before warning (use {} for multiple lines)
  * \param expression static_cast<bool>(expression) == true -> warning
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_WARN(expression, message, ...) \
    CTH_WARN_T(cth::except::default_exception, expression, message, __VA_ARGS__)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_WARN

#undef CTH_INFO_T
#undef CTH_INFO
/**
  * \brief can execute code before info (use {} for multiple lines)
  * \param type exception type
  * \param expression static_cast<bool>(expression) == true -> inform
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_INFO_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::INFO, expression, message, __VA_ARGS__)
/**
  * \brief can execute code before info (use {} for multiple lines)
  * \param expression static_cast<bool>(expression) == true -> inform
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_INFO(expression, message, ...) CTH_INFO_T(cth::except::default_exception, expression, message, __VA_ARGS__)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_INFO

#undef CTH_LOG_T
#undef CTH_LOG
/**
  * \brief can execute code before log (use {} for multiple lines)
  * \param type exception type
  * \param expression static_cast<bool>(expression) == true -> log
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_LOG_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::LOG, expression, message, __VA_ARGS__)
/**
  * \brief can execute code before log (use {} for multiple lines)
  * \param expression static_cast<bool>(expression) == true -> log
  * \param message std::format_string
  * \param ... std::format arguments
  * \note this macro MUST be followed by ; or {}
  * \note #ifndef _DEBUG -> disabled
  */
#define CTH_LOG(expression, message, ...) CTH_LOG_T(cth::except::default_exception, expression, message, __VA_ARGS__)

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

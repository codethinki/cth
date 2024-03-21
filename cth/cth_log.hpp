#pragma once

// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
// disabled because this header is forced to use macros
#include "cth_console.hpp"
#include "cth_exception.hpp"

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

namespace cth::log {


namespace dev {
    inline static bool colored = true;
    inline static out::col_stream logStream{&std::cout, out::error.state()};

    static constexpr out::Text_Colors textColor(const cth::except::Severity severity) {
        switch(severity) {
            case cth::except::LOG:
                return out::WHITE_TEXT_COL;
            case cth::except::Severity::INFO:
                return out::DARK_CYAN_TEXT_COL;
            case cth::except::Severity::WARNING:
                return out::DARK_YELLOW_TEXT_COL;
            case cth::except::Severity::ERR:
                return out::DARK_RED_TEXT_COL;
            case cth::except::Severity::CRITICAL:
                return out::DARK_RED_TEXT_COL;
            default:
                return out::WHITE_TEXT_COL;
        }
    }
} // namespace dev

/**
 * \brief sets a colored log stream. nullptr -> colored console output
 */
inline void setLogStream(const out::col_stream& stream) {
    dev::colored = true;
    dev::logStream = stream;
}
inline void setLogStream(std::ostream* stream) {
    dev::colored = false;
    dev::logStream = out::col_stream{stream};
}



template<typename... Types> std::enable_if_t<(sizeof...(Types) > 0u), void>
msg(const cth::except::Severity severity, std::format_string<Types...> f_str, Types&&... types) {
    if(severity < CTH_LOG_LEVEL) return;
    if(dev::colored) dev::logStream.println(dev::textColor(severity), std::format(f_str, std::forward<Types>(types)...));
    else dev::logStream.println(std::format(f_str, std::forward<Types>(types)...));
}
inline void msg(const cth::except::Severity severity, const std::string_view message) {
    if(severity < CTH_LOG_LEVEL) return;
    if(dev::colored) dev::logStream.println(dev::textColor(severity), message);
    else dev::logStream.println(message);
}
template<cth::except::Severity S = cth::except::LOG, typename... Types> std::enable_if_t<(sizeof...(Types) > 0u), void>
msg(std::format_string<Types...> f_str, Types&&... types) {
    if constexpr (S < CTH_LOG_LEVEL) return;
    if(dev::colored) dev::logStream.println(dev::textColor(S), std::format(f_str, std::forward<Types>(types)...));
    else dev::logStream.println(std::format(f_str, std::forward<Types>(types)...));
}
template<cth::except::Severity S = cth::except::LOG>
inline void msg(const std::string_view message) {
    if constexpr (S < CTH_LOG_LEVEL) return;
    if(dev::colored) dev::logStream.println(dev::textColor(S), message);
    else dev::logStream.println(message);
}



namespace dev {
    /**
     * \brief executes the statement in its destructor to support code execution before aborting
     */
    template<cth::except::Severity S>
    struct LogObj {
        explicit LogObj(cth::except::default_exception exception) : ex(std::move(exception)), msgVerbosity(S) {}
        ~LogObj() {
            if constexpr(static_cast<int>(S) < CTH_LOG_LEVEL) return;

            std::string out = "\n";
            switch(msgVerbosity) {
                case except::CRITICAL:
                    out += ex.string();
                    break;
                case except::ERR:
                    out += ex.string();
                    break;
                case except::WARNING:
                    out += std::format("{0} {1} {2} {3}", ex.what(), ex.details(), ex.func_string(), ex.loc_string());
                    break;
                case except::INFO:
                    out += std::format("{0} {1} {2}", ex.what(), ex.details(), ex.func_string());
                    break;
                case except::LOG:
                    out = std::format("{0} {1}", ex.what(), ex.details());
                    break;
            }
            cth::log::msg(S, out);

            if constexpr(S == cth::except::Severity::CRITICAL) std::terminate();
        }
        void add(const std::string_view message) { ex.add(message.data()); }
        template<typename... Types> std::enable_if_t<(sizeof...(Types) > 0u), void> add(const std::format_string<Types...> f_str, Types&&... types) {
            ex.add(f_str, std::forward<Types>(types)...);
        }

        void setVerbosity(const cth::except::Severity new_verbosity) { msgVerbosity = new_verbosity; }

    private:
        cth::except::default_exception ex;
        cth::except::Severity msgVerbosity;

    public:
        [[nodiscard]] cth::except::Severity verbosity() const { return msgVerbosity; }
        [[nodiscard]] std::string string() const { return ex.string(); }
        [[nodiscard]] cth::except::default_exception exception() const { return ex; }
    };



    /**
     * \brief wrapper for dev::LogObj
     * \param expression (expression) == false -> code execution + delayed log message
     * \param message log message
     * \param severity log severity
     */
#define CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message_str, severity) \
    if(const auto details =\
        (static_cast<bool>(expression) ? std::make_unique<cth::log::dev::LogObj<severity>>(cth::except::default_exception{severity, message_str,\
        std::source_location::current(), std::stacktrace::current()}) : nullptr);\
        static_cast<bool>(expression)) //{...}
#define CTH_DEV_DISABLED_LOG_TEMPLATE() if(std::unique_ptr<cth::log::dev::LogObj<cth::except::Severity::LOG>> details = nullptr; false) //{...}


} // namespace dev


//------------------------------
//  CTH_STABLE_ASSERTION
//------------------------------
/**
 * \brief assert extension\n
 * stable -> also for _NDEBUG\n
 * can execute code before abort (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}
 * \param (expression) == false -> error
 */
#define CTH_STABLE_ASSERT(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(!(expression), message, cth::except::Severity::CRITICAL)

/**
 * \brief if(expression) -> error msg\n
 * can execute code before aborting (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}
 * \param (expression) == false -> abort
 */
#define CTH_STABLE_ABORT(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::CRITICAL)

/**
 * \brief if(expression) -> error msg\n
 * can execute code before warning (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}
 * \param (expression) == false -> error
 */
#define CTH_STABLE_ERR(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::ERR)

/**
 * \brief if(expression) -> warn\n
 * stable -> also for _NDEBUG\n
 * can execute code before warning (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}
 * \param (expression) == false -> warn
 */
#define CTH_STABLE_WARN(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::WARNING)

/**
 * \brief if(expression) -> hint\n
 *  stable -> also for _NDEBUG\n
 * can execute code before hinting (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}
 * \param (expression) == false -> hint
 */
#define CTH_STABLE_INFORM(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::INFO)

/**
 * \brief if(expression) -> log message\n
 * stable -> also for _NDEBUG\n
 * can execute code before logging (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}
 * \param (expression) == false -> log
 */
#define CTH_STABLE_LOG(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::LOG)


//------------------------------
//        CTH_ASSERTION
//------------------------------
#define CTH_ASSERT(expression, message) CTH_DEV_DISABLED_LOG_TEMPLATE()
#define CTH_ABORT(expression, message) CTH_DEV_DISABLED_LOG_TEMPLATE()

#define CTH_ERR(expression, message) CTH_DEV_DISABLED_LOG_TEMPLATE()
#define CTH_WARN(expression, message) CTH_DEV_DISABLED_LOG_TEMPLATE()
#define CTH_INFORM(expression, message) CTH_DEV_DISABLED_LOG_TEMPLATE()
#define CTH_LOG(expression, message) CTH_DEV_DISABLED_LOG_TEMPLATE()


#ifdef _DEBUG

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_NONE

#undef CTH_ASSERT
/**
 * \brief assert extension\n
 * can execute code before abort (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}\n
 * \note \note disabled when #ifndef _DEBUG or #if CTH_LOG_LEVEL > CTH_LOG_LEVEL_CRITICAL
 * \param (expression) == false -> error msg -> abort
 */
#define CTH_ASSERT(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(!(expression), message, cth::except::Severity::CRITICAL)


#undef CTH_ABORT
/**
 * \brief if(expression) -> error msg\n
 * can execute code before warning (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}\n
 * \note \note disabled when #ifndef _DEBUG or #if CTH_LOG_LEVEL > CTH_LOG_LEVEL_CRITICAL
 * \param (expression) == false -> error
 */
#define CTH_ABORT(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::CRITICAL)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_CRITICAL

#undef CTH_ERR
/**
 * \brief if(expression) -> error msg\n
 * can execute code before warning (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}\n
 * \note \note disabled when #ifndef _DEBUG or #if CTH_LOG_LEVEL > CTH_LOG_LEVEL_ERR
 * \param (expression) == false -> error
 */
#define CTH_ERR(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::ERR)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_ERR


#undef CTH_WARN
/**
 * \brief if(expression) -> warn\n
 * can execute code before warning (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}\n
 * \note \note disabled when #ifndef _DEBUG or #if CTH_LOG_LEVEL > CTH_LOG_LEVEL_WARN
 * \param (expression) == false -> warn
 */
#define CTH_WARN(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::WARNING)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_WARN

#undef CTH_INFORM
/**
 * \brief if(expression) -> hint\n
 * can execute code before hinting (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}\n
 * \note \note disabled when #ifndef _DEBUG or #if CTH_LOG_LEVEL > CTH_LOG_LEVEL_INFO
 * \param (expression) == false -> hint
 */
#define CTH_INFORM(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::INFO)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_INFO

#undef CTH_LOG
/**
 * \brief if(expression) -> log message\n
 * can execute code before logging (use {} for multiple lines)\n
 * \note this macro MUST be followed by ; or {}\n
 * \note \note disabled when #ifndef _DEBUG or #if CTH_LOG_LEVEL > CTH_LOG_LEVEL_LOG
 * \param (expression) == false -> log
 */
#define CTH_LOG(expression, message) CTH_DEV_DELAYED_LOG_TEMPLATE(expression, message, cth::except::Severity::LOG)

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

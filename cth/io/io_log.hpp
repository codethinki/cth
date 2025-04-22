#pragma once
#include "../mode.hpp"
// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
// disabled because this header is forced to use macros

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


/**
 * \brief wrapper for dev::LogObj
 * \param expression (expression) == false -> code execution + delayed log message
 * \param message log message
 * \param severity log severity
 */
#define CTH_DEV_DELAYED_LOG_TEMPLATE(severity, expr, message_str, ...) \
     if(\
        auto const details = static_cast<bool>(expr)\
            ? std::make_unique<cth::log::LogObj<severity>>(\
                cth::except::default_exception{\
                    std::format(message_str, __VA_ARGS__),\
                    severity,\
                    std::source_location::current(),\
                    std::stacktrace::current()\
                }\
              )\
            : nullptr;\
         static_cast<bool>(expr)\
        ) [[unlikely]]

#define CTH_DEV_DISABLED_CRITICAL_TEMPLATE(expr) \
    /*TEMP currently not supported by msvc[[assume(!static_cast<bool>(expr))]]; \*/\
    __assume(!static_cast<bool>(expr));\
    if(std::unique_ptr<cth::log::LogObj<cth::except::Severity::CRITICAL>> details = nullptr; static_cast<bool>(expr)) [[unlikely]]

#define CTH_DEV_DISABLED_LOG_TEMPLATE() if(std::unique_ptr<cth::log::LogObj<cth::except::Severity::LOG>> details = nullptr; false) //{...}



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
#define CTH_STABLE_LOG(expression, message, ...) CTH_DEV_DELAYED_LOG_TEMPLATE(cth::except::Severity::LOG, expression, message, __VA_ARGS__)


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

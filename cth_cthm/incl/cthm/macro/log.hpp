#pragma once
#include "compilation.hpp"
#include "log_details/log_level.hpp"

#include <format>
#include <memory>


import cth.exception;
import cth.io;

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

//------------------------------
//  CTH_STABLE_LOGS
//------------------------------

#define CTH_STABLE_ASSERT_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::CRITICAL, !(expression), message, __VA_ARGS__)

#define CTH_STABLE_ASSERT(expression, message, ...) \
    CTH_STABLE_ASSERT_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_STABLE_ABORT_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::CRITICAL, expression, message, __VA_ARGS__)

#define CTH_STABLE_ABORT(expression, message, ...) \
    CTH_STABLE_ABORT_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_STABLE_ERR_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::ERR, expression, message, __VA_ARGS__)

#define CTH_STABLE_ERR(expression, message, ...) \
    CTH_STABLE_ERR_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_STABLE_WARN_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::WARNING, expression, message, __VA_ARGS__)

#define CTH_STABLE_WARN(expression, message, ...) \
    CTH_STABLE_WARN_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_STABLE_INFO_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::INFO, expression, message, __VA_ARGS__)

#define CTH_STABLE_INFO(expression, message, ...) \
    CTH_STABLE_INFO_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_STABLE_LOG_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::LOG, expression, message, __VA_ARGS__)

#define CTH_STABLE_LOG(expression, message, ...) \
    CTH_STABLE_LOG_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#define CTH_STABLE_THROW_T(type, expression, message, ...) \
    CTH_DEV_LOG_AUTO_THROW_TEMPLATE_T(type, cth::except::Severity::ERR, expression, message, __VA_ARGS__)

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
#define CTH_ASSERT_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::CRITICAL, !(expression), message, __VA_ARGS__)
#define CTH_ASSERT(expression, message, ...) \
    CTH_ASSERT_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#undef CTH_CRITICAL_T
#undef CTH_CRITICAL
#define CTH_CRITICAL_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::CRITICAL, expression, message, __VA_ARGS__)
#define CTH_CRITICAL(expression, message, ...) \
    CTH_CRITICAL_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_CRITICAL

#undef CTH_THROW_T
#undef CTH_THROW
#define CTH_THROW_T(type, expression, message, ...) \
    CTH_DEV_LOG_AUTO_THROW_TEMPLATE_T(type, cth::except::Severity::ERR, expression, message, __VA_ARGS__)
#define CTH_THROW(expression, message, ...) \
    CTH_THROW_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#undef CTH_ERR_T
#undef CTH_ERR
#define CTH_ERR_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::ERR, expression, message, __VA_ARGS__)
#define CTH_ERR(expression, message, ...) CTH_ERR_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_ERR

#undef CTH_WARN_T
#undef CTH_WARN
#define CTH_WARN_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::WARNING, expression, message, __VA_ARGS__)
#define CTH_WARN(expression, message, ...) \
    CTH_WARN_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_WARN

#undef CTH_INFO_T
#undef CTH_INFO
#define CTH_INFO_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::INFO, expression, message, __VA_ARGS__)
#define CTH_INFO(expression, message, ...) CTH_INFO_T(cth::except::default_exception, expression, message, __VA_ARGS__)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_INFO

#undef CTH_LOG_T
#undef CTH_LOG
#define CTH_LOG_T(type, expression, message, ...) \
    CTH_DEV_DELAYED_LOG_TEMPLATE_T(type, cth::except::Severity::LOG, expression, message, __VA_ARGS__)
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


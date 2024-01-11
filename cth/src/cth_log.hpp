#pragma once
// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
// disabled because this header is forced to use macros

#include <assert.h>

#include "cth_concepts.hpp"
#include "cth_console.hpp"

#define CTH_LOG_LEVEL_ALL 0
#define CTH_LOG_LEVEL_DEBUG 0
#define CTH_LOG_LEVEL_INFO 1
#define CTH_LOG_LEVEL_WARN 2
#define CTH_LOG_LEVEL_ERROR 3
#define CTH_LOG_LEVEL_NONE 4

#ifndef CTH_LOG_LEVEL
#define CTH_LOG_LEVEL CTH_LOG_LEVEL_ALL
#endif

namespace cth::log {
enum Cth_Log_Colors {
    LOG_COLOR_PASSED = console::COL_ID_DARK_GREEN_TEXT,
    LOG_COLOR_FAILED = console::COL_ID_BRIGHT_RED_TEXT,
    LOG_COLOR_DEFAULT = console::COL_ID_WHITE_TEXT,
    LOG_COLOR_HINT = console::COL_ID_DARK_CYAN_TEXT,
    LOG_COLOR_WARN = console::COL_ID_DARK_YELLOW_TEXT,
    LOG_COLOR_ABORT = console::COL_ID_DARK_RED_TEXT
};

inline void msg(const std::string_view msg, Cth_Log_Colors err_severity = LOG_COLOR_DEFAULT) {
    console::print(msg, static_cast<console::Text_Col_Ids>(err_severity));
}

inline void msg(const std::wstring& msg, const std::wstring& file, const Cth_Log_Colors err_type) {
    console::errln(msg + L", file: " + file + L'\n', static_cast<console::Text_Col_Ids>(err_type));
}


namespace dev {
    /**
     * \brief executes the statement in its destructor to support code execution before aborting
     */
    template<Cth_Log_Colors LogType>
    struct DelayedLogObj {
        DelayedLogObj(const bool expression, std::wstring msg, std::wstring file) : expression(expression), msg(std::move(msg)),
            file(std::move(file)) {}
        ~DelayedLogObj() {
            if(expression) return;
            std::wstring message{};

            if constexpr(LogType == LOG_COLOR_DEFAULT) message = L"LOG: ";
            else if constexpr(LogType == LOG_COLOR_HINT) message = L"HINT: ";
            else if constexpr(LogType == LOG_COLOR_WARN) message = L"WARNING: ";
            else if constexpr(LogType == LOG_COLOR_ABORT) message = L"ERROR: ";
            else
                assert(false && "invalid log type");
            log::msg(message + msg, file, LogType);


            //----------------------------------------
            //         CTH_ASSERTION_ERROR
            //----------------------------------------
            if constexpr(LogType == LOG_COLOR_ABORT) abort();
        }

    private:
        bool expression;
        const std::wstring msg;
        const std::wstring file;
    };
}

#define CTH_TO_WIDE_STR2(s) L ## s
#define CTH_TO_WIDE_STR(s) CTH_TO_WIDE_STR2(s)


/**
 * \brief uses an if init statement with the dev::DelayedLogObj to support delayed warning, hint, log and assertion
 * \param expression expression false -> delayed code execution
 * \param file path details string
 */
#define CTH_DEV_DELAYED_STATEMENT_TEMPLATE(expression, file, log_type) if(cth::log::dev::DelayedLogObj<log_type> logObject{(expression), CTH_TO_WIDE_STR(#expression), file}; !(expression))

#define CTH_ASSERT(expression) if(false)
#define CTH_WARN(expression) if(false)
#define CTH_HINT(expression) if(false)
#define CTH_LOG(expression) if(false)
#define CTH_TEST(expression) if(false)


#ifndef _NDEBUG

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_NONE

#undef CTH_ASSERT
/**
 * \brief This is an assert extension that can execute code before aborting (use {} for multiple lines)\n
 * CAUTION: THIS MACRO IS AN IMPLICIT IF STATEMENT\n
 * FOLLOW WITH ';' IF NOT REQUIRED
 * \param expression false -> assertion error
 */
#define CTH_ASSERT(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_ABORT)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_ERROR

#undef CTH_WARN
/**
 * \brief This is an assert extension without abort that can execute code before warning (use {} for multiple lines)\n
 * CAUTION: THIS MACRO IS AN IMPLICIT IF STATEMENT\n
 * FOLLOW WITH ';' IF NOT REQUIRED
 * \param expression false -> warning
 */
#define CTH_WARN(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_WARN)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_WARN

#undef CTH_HINT
/**
 * \brief This is an assert extension without abort that can execute code before hinting (use {} for multiple lines)\n
 * CAUTION: THIS MACRO IS AN IMPLICIT IF STATEMENT\n
 * FOLLOW WITH ';' IF NOT REQUIRED
 * \param expression false -> hint
 */
#define CTH_HINT(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_HINT)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_INFO

#undef CTH_LOG
/**
 * \brief This is an assert extension without abort that can execute code before logging (use {} for multiple lines)\n
 * CAUTION: THIS MACRO IS AN IMPLICIT IF STATEMENT\n
 * FOLLOW WITH ';' IF NOT REQUIRED
 * \param expression false -> log
 */
#define CTH_LOG(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_DEFAULT)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_DEBUG
    inline auto x = []() {
        CTH_ASSERT(false && "invalid CTH_LOG_LEVEL macro value defined");
        return 10;
        }();
#endif

#endif
#endif
#endif
#endif

#endif
}

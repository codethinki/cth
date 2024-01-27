#pragma once
// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
// disabled because this header is forced to use macros

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

namespace dev {
    inline static std::wostream* logStream = nullptr;

    inline void msg(const std::wstring& msg, const std::wstring& file, const Cth_Log_Colors err_type) {
        console::errln(msg + L", file: " + file + L'\n', static_cast<console::Text_Col_Ids>(err_type));
    }

    /**
     * \brief executes the statement in its destructor to support code execution before aborting
     */
    template<Cth_Log_Colors LogType>
    struct DelayedLogObj {
        DelayedLogObj(const bool expression, std::wstring msg, std::wstring file) : expression(expression),
        msg(std::move(msg)), file(std::move(file)) {}
        ~DelayedLogObj() {
            if(expression) return;

            if constexpr(LogType == LOG_COLOR_DEFAULT) msg = L"LOG: " + msg;
            else if constexpr(LogType == LOG_COLOR_HINT) msg = L"HINT: " + msg;
            else if constexpr(LogType == LOG_COLOR_WARN) msg = L"WARNING: " + msg;
            else if constexpr(LogType == LOG_COLOR_ABORT) msg = L"ERROR: " + msg;

            if(logStream == nullptr) log::dev::msg(msg, file, LogType);
            else *logStream << msg << L", file: " << file << L'\n';

            //----------------------------------------
            //         CTH_ASSERTION_ERROR
            //----------------------------------------
            if constexpr(LogType == LOG_COLOR_ABORT) abort();
        }

    private:
        bool expression;
        std::wstring msg;
        const std::wstring file;
    };
}


inline void msg(const std::string_view msg, Cth_Log_Colors err_severity = LOG_COLOR_DEFAULT) {
    console::print(msg, static_cast<console::Text_Col_Ids>(err_severity));
}



/**
 * \brief sets the log stream. nullptr -> colored console output
 * \param stream 
 */
inline void setLogStream(std::wostream* stream) { dev::logStream = stream; }

#define CTH_TO_WIDE_STR2(s) L ## s
#define CTH_TO_WIDE_STR(s) CTH_TO_WIDE_STR2(s)


/**
 * \brief wrapper for dev::DelayedLogObj
 * \param expression expression false -> delayed code execution
 * \param file path details string
 */
#define CTH_DEV_DELAYED_STATEMENT_TEMPLATE(expression, file, log_type) if(cth::log::dev::DelayedLogObj<log_type> logObject{(expression), CTH_TO_WIDE_STR(#expression), file}; !(expression))

//------------------------------
//  CTH_STABLE_ASSERTION
//------------------------------


#define CTH_STABLE_ASSERT(expression) if(false)
#define CTH_STABLE_WARN(expression) if(false)
#define CTH_STABLE_HINT(expression) if(false)
#define CTH_STABLE_LOG(expression) if(false)

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_NONE

#undef CTH_STABLE_ASSERT
/**
 * \brief assert extension\n
 * stable -> also for _NDEBUG\n
 * can execute code (use {} for multiple lines)\n
 * CAUTION: THIS MACRO MUST BE TERMINATED WITH ';' or {}
 * \param expression false -> error
 */
#define CTH_STABLE_ASSERT(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_ABORT)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_ERROR

#undef CTH_STABLE_WARN
/**
 * \brief if(!expression) -> warn\n
 * stable -> also for _NDEBUG\n
 * can execute code before warning (use {} for multiple lines)\n
 * CAUTION: THIS MACRO MUST BE TERMINATED WITH ';' or {}
 * \param expression false -> warn
 */
#define CTH_STABLE_WARN(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_WARN)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_WARN

#undef CTH_STABLE_HINT
/**
 * \brief if(!expression) -> hint\n
 *  stable -> also for _NDEBUG\n
 * can execute code before hinting (use {} for multiple lines)\n
 * CAUTION: THIS MACRO MUST BE TERMINATED WITH ';' or {}
 * \param expression false -> hint
 */
#define CTH_STABLE_HINT(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_HINT)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_INFO

#undef CTH_STABLE_LOG
/**
 * \brief if(!expression) -> log message\n
 * stable -> also for _NDEBUG\n
 * can execute code before logging (use {} for multiple lines)\n
 * CAUTION: THIS MACRO MUST BE TERMINATED WITH ';' or {}
 * \param expression false -> log
 */
#define CTH_STABLE_LOG(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_DEFAULT)

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


//------------------------------
//        CTH_ASSERTION
//------------------------------
#define CTH_ASSERT(expression) if(false)
#define CTH_WARN(expression) if(false)
#define CTH_HINT(expression) if(false)
#define CTH_LOG(expression) if(false)


#ifdef _DEBUG

#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_NONE

#undef CTH_ASSERT
/**
 * \brief assert extension\n
 * can execute code (use {} for multiple lines)\n
 * CAUTION: THIS MACRO MUST BE TERMINATED WITH ';' or {}
 * \param expression false -> error
 */
#define CTH_ASSERT(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_ABORT)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_ERROR

#undef CTH_WARN
/**
 * \brief if(!expression) -> warn\n
 * can execute code before warning (use {} for multiple lines)\n
 * CAUTION: THIS MACRO MUST BE TERMINATED WITH ';' or {}
 * \param expression false -> warn
 */
#define CTH_WARN(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_WARN)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_WARN

#undef CTH_HINT
/**
 * \brief if(!expression) -> hint\n
 * can execute code before hinting (use {} for multiple lines)\n
 * CAUTION: THIS MACRO MUST BE TERMINATED WITH ';' or {}
 * \param expression false -> hint
 */
#define CTH_HINT(expression) CTH_DEV_DELAYED_STATEMENT_TEMPLATE((expression), CTH_TO_WIDE_STR(__FILE__), cth::log::Cth_Log_Colors::LOG_COLOR_HINT)
#if CTH_LOG_LEVEL != CTH_LOG_LEVEL_INFO

#undef CTH_LOG
/**
 * \brief if(!expression) -> log message\n
 * can execute code before logging (use {} for multiple lines)\n
 * CAUTION: THIS MACRO MUST BE TERMINATED WITH ';' or {}
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
} // namespace cth::log

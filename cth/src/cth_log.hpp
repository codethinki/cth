#pragma once
//TEMP until cth_console is implemented
//#include <iostream>
//#include <string>
//#include "cth_type_traits.hpp"
//
//
//import hlc.console;
//import hlc.type_traits;
//
//#define HLC_LOG_LEVEL_ALL 0
//#define HLC_LOG_LEVEL_DEBUG 0
//#define HLC_LOG_LEVEL_INFO 1
//#define HLC_LOG_LEVEL_WARN 2
//#define HLC_LOG_LEVEL_ERROR 3
//#define HLC_LOG_LEVEL_NONE 4
//
//#ifndef HLC_LOG_LEVEL
//#define HLC_LOG_LEVEL HLC_LOG_LEVEL_ALL
//#endif
//
//namespace hlc::log {
//    enum Hlc_Log_Colors {
//        LOG_COLOR_PASSED = console::ANSI_ID_DARK_GREEN,
//        LOG_COLOR_FAILED = console::ANSI_ID_BRIGHT_RED,
//        LOG_COLOR_DEFAULT = console::ANSI_ID_WHITE,
//        LOG_COLOR_HINT = console::ANSI_ID_DARK_CYAN,
//        LOG_COLOR_WARN = console::ANSI_ID_DARK_YELLOW,
//        LOG_COLOR_ABORT = console::ANSI_ID_DARK_RED,
//    };
//
//    template<typename T> std::enable_if_t<type::is_textual_v<T>, void>
//    msg(const T& msg, Hlc_Log_Colors err_type = LOG_COLOR_DEFAULT) { console::errln(msg, static_cast<console::Ansi_Color_Ids>(err_type)); }
//
//    inline void msg(const std::wstring& msg, const std::wstring& file, const unsigned line, const Hlc_Log_Colors err_type) {
//        console::errln(msg + L", file: " + file + L", line: " + std::to_wstring(line), static_cast<console::Ansi_Color_Ids>(err_type));
//    }
//
//}
//
//
//#define _TO_WIDE_STR(s) L ## s
//#define TO_WIDE_STR(s) _TO_WIDE_STR(s)
//
//#define HLC_LOG_FUNC_TEMPLATE(expression, log_color) (void) ((!!(expression)) ||\
//            (hlc::log::msg(TO_WIDE_STR(#expression), TO_WIDE_STR(__FILE__), (unsigned)(__LINE__), log_color), 0), 0)
//
//#define HLC_LOG_ERROR_FUNC(expression) (void)((!!(expression)) ||\
//            (hlc::log::msg(TO_WIDE_STR(#expression), TO_WIDE_STR(__FILE__), (unsigned)(__LINE__), hlc::log::LOG_COLOR_ABORT), 0), 0);\
//            (void)((!!(expression)) || (abort(), 0))
//#define HLC_LOG_CHECK_FUNC(expression) (void)((hlc::log::msg(TO_WIDE_STR(#expression), TO_WIDE_STR(__FILE__), (unsigned)(__LINE__),\
//    expression ? hlc::log::LOG_COLOR_PASSED : hlc::log::LOG_COLOR_FAILED), 0), 0)
//
//
//
//#define ABORT(expression) ((void)0)
//#define WARN(expression) ((void)0)
//#define HINT(expression) ((void)0)
//#define LOG(expression) ((void)0)
//#define TEST(expression) ((void)0)
//
//
//#ifndef _NDEBUG
//
//#if HLC_LOG_LEVEL != HLC_LOG_LEVEL_NONE
//
//#undef ABORT
//#define ABORT(expression) HLC_LOG_ERROR_FUNC(expression)
//#if HLC_LOG_LEVEL != HLC_LOG_LEVEL_ERROR
//
//#undef WARN
//#define WARN(expression) HLC_LOG_FUNC_TEMPLATE(expression, hlc::log::LOG_COLOR_WARN)
//#if HLC_LOG_LEVEL != HLC_LOG_LEVEL_WARN
//
//#undef HINT
//#define HINT(expression) HLC_LOG_FUNC_TEMPLATE(expression, hlc::log::LOG_COLOR_HINT)
//#if HLC_LOG_LEVEL != HLC_LOG_LEVEL_INFO
//
//#undef LOG
//#define LOG(expression) HLC_LOG_FUNC_TEMPLATE(expression, hlc::log::LOG_COLOR_DEFAULT)
//
//#undef CHECK
//#define CHECK(expression) HLC_LOG_CHECK_FUNC(expression)
//
//#if HLC_LOG_LEVEL != HLC_LOG_LEVEL_DEBUG
//inline auto x = []() {
//    ABORT(false && "invalid HLC_LOG_LEVEL macro value defined");
//    return 10;
//    }();
//#endif
//
//#endif
//#endif
//#endif
//#endif
//
//#endif
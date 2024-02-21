#pragma once
// ReSharper disable CppClangTidyModernizePassByValue
//bc exceptions need to copy the data
#include <filesystem>
#include <source_location>
#include<stacktrace>
#include <string>

namespace cth::except {
using namespace std;

enum Severity {
    LOG,
    INFO,
    WARNING,
    ERR,
    CRITICAL
};
constexpr static std::string_view to_string(const Severity sev) {
    switch (sev) {
    case LOG:
        return "LOG: ";
    case INFO:
        return "INFO: ";
    case WARNING:
        return "WARNING: ";
    case ERR:
        return "ERROR: ";
    case CRITICAL:
        return "CRITICAL ERROR:";
    }
    return "UNKNOWN: ";
}


template<Severity S>
class default_exception : public std::exception{
public:

    explicit default_exception(std::string msg, std::source_location loc = std::source_location::current(),
        std::stacktrace trace = std::stacktrace::current()) : logMsg{std::string(to_string(S)) + ' ' + msg + '\n'},
        loc{loc}, trace{(trace)} {}


    [[nodiscard]] static constexpr Severity severity() { return S; }
    [[nodiscard]] const char* what() const override { return logMsg.c_str(); }

    [[nodiscard]] std::stacktrace stacktrace() const { return trace; }
    [[nodiscard]] std::source_location location() const { return loc; }

    [[nodiscard]] std::string string() const { return std::format("{0} {1} {2}", logMsg, loc_string(), trace_string()); }
    [[nodiscard]] std::string loc_string() const {
        return std::format("FUNCTION: {0}\n LOCATION: \"{1}({2}:{3})\"\n", loc.function_name(), loc.file_name(), loc.line(), loc.column());
    }
    [[nodiscard]] std::string trace_string() const {
        std::string str = "STACKTRACE:\n";

        std::for_each(trace.begin(), trace.end() - 2, [&](const auto& entry) {
            const std::filesystem::path path{entry.source_file()};
            str += std::format("\t{2} : {0}({1})\n", path.filename().string(), entry.source_line(), entry.description());
        });
        return str;
    }

private:
    std::string logMsg;
    std::source_location loc;
    std::stacktrace trace;
};
template<Severity S, typename T>
class data_exception : public default_exception<S> {
public:
    data_exception(std::string msg, T data, std::source_location loc = std::source_location::current(),
        std::stacktrace trace = std::stacktrace::current()) : default_exception<S>{std::move(msg), std::move(loc), std::move(trace)},
        dataObj{std::move(data)} {}
    data_exception(T data, default_exception<S> exception) : default_exception<S>{exception}, dataObj{data} {}
    [[nodiscard]] T data() const { return dataObj; }

private:
    T dataObj;
};


}


inline std::ostream& operator <<(std::ostream& os, const std::source_location& loc) {
    os << std::format("FUNCTION: {0}\n LOCATION: '{1}({2}:{3})'\n", loc.function_name(), loc.file_name(), loc.line(), loc.column());
    return os;
}

inline std::ostream& operator <<(std::ostream& os, const std::stacktrace& backtrace) {
    for(auto it = backtrace.begin(); it != backtrace.end() - 3; ++it)
        os << it->source_file() << "(" << it->source_line() << ") : " << it->description() << "\n";
    return os;
}

#pragma once
// ReSharper disable CppPassValueParameterByConstReference
// ReSharper disable CppClangTidyModernizePassByValue
//exception throw causes stack unwind -> dangling reference
#include <filesystem>
#include <source_location>
#include<stacktrace>
#include <string>

namespace cth::except {


enum Severity {
    LOG,
    INFO,
    WARNING,
    ERR,
    CRITICAL,
    SEVERITY_SIZE,
};
constexpr static std::string_view to_string(Severity const sev) {
    switch(sev) {
        case LOG:
            return "LOG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERR:
            return "ERROR";
        case CRITICAL:
            return "CRITICAL ERROR";
        default:
            std::unreachable();
    }
    return "UNKNOWN";
}


class default_exception : public std::exception {
public:
    explicit default_exception(std::string msg, Severity const severity = cth::except::ERR,
        std::source_location loc = std::source_location::current(),
        std::stacktrace trace = std::stacktrace::current()) : _severity(severity), _msg{std::format(" {0}\n", msg)},
        _sourceLocation{loc}, _trace{std::move(trace)} {}


    void add(std::string msg) noexcept {
        addNoCpy(msg);
    }
    template<typename... Args> requires (sizeof...(Args) > 0u)
    void add(std::format_string<Args...> const f_str, Args&&... types) noexcept { this->addNoCpy(std::format(f_str, std::forward<Args>(types)...)); }



    [[nodiscard]] std::string string() const noexcept {
        return std::format("{0} {1} {2} {3} {4}", _msg, _details, func_string(), loc_string(), trace_string());
    }
    [[nodiscard]] std::string brief() const noexcept {
        return std::format("{0} {1} {2}", _msg, func_string(), loc_string());
    }

    [[nodiscard]] std::string loc_string() const noexcept {
        std::string const filename = std::string(_sourceLocation.file_name());
        return std::format("LOCATION: {0}({1}:{2})\n", filename.substr(filename.find_last_of('\\') + 1),
            _sourceLocation.line(), _sourceLocation.column());
    }
    [[nodiscard]] std::string func_string() const noexcept { return std::format("FUNCTION: {0}\n", _sourceLocation.function_name()); }
    [[nodiscard]] std::string trace_string() const noexcept {
        std::string str = "STACKTRACE:\n";

        std::for_each(_trace.begin(), _trace.end() - 2, [&](auto const& entry) {
            std::filesystem::path const path{entry.source_file()};
            str += std::format("\t{2} : {0}({1})\n", path.filename().string(), entry.source_line(), entry.description());
        });
        return str;
    }





private:
    void addNoCpy(std::string const& msg) noexcept {
        if(_details.empty()) _details = "DETAILS:\n";
        _details += '\t';
        _details += msg;
        _details += '\n';

        _what = string();
    }

    Severity _severity;
    std::string _msg;
    std::string _details{};
    std::source_location _sourceLocation;
    std::stacktrace _trace;

    std::string _what{};

public:
    [[nodiscard]] Severity severity() const noexcept { return _severity; }
    [[nodiscard]] std::string details() const noexcept { return _details; }
    [[nodiscard]] std::string msg() const noexcept { return _msg; }

    [[nodiscard]] std::stacktrace stacktrace() const noexcept { return _trace; }
    [[nodiscard]] std::source_location location() const noexcept { return _sourceLocation; }

    [[nodiscard]] char const* what() const noexcept override { return _what.c_str(); }

};
template<typename T>
class data_exception : public default_exception {
public:
    data_exception(std::string msg, T data, Severity const severity = cth::except::ERR, std::source_location loc = std::source_location::current(),
        std::stacktrace trace = std::stacktrace::current()) : default_exception{std::move(msg), severity, loc, std::move(trace)},
        _dataObj{std::move(data)} {}
    data_exception(T data, default_exception exception) : default_exception{std::move(exception)}, _dataObj{std::move(data)} {}
    [[nodiscard]] T data() const noexcept { return _dataObj; }

private:
    T _dataObj;
};


} // namespace cth::except

#pragma once

// ReSharper disable CppClangTidyPerformanceUnnecessaryValueParam
// ReSharper disable CppClangTidyBugproneExceptionEscape
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
constexpr static std::string_view to_string(Severity sev) {
    switch(sev) {
        // NOLINT(clang-diagnostic-switch-enum)
        case LOG: return "LOG";
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERR: return "ERROR";
        case CRITICAL: return "CRITICAL ERROR";
        default: std::unreachable();
    }
    return "UNKNOWN";
}
}


CTH_FORMAT_TYPE(cth::except::Severity, cth::except::to_string);


namespace cth::except {
class default_exception : public std::exception {
public:
    explicit default_exception(std::string msg, Severity const severity = cth::except::ERR,
        std::source_location loc = std::source_location::current(),
        std::stacktrace trace = std::stacktrace::current()) : _severity(severity), _msg{std::format(" {0}\n", msg)},
        _sourceLocation{loc}, _trace{std::move(trace)}, _what{string()} {}


    default_exception& add(std::string msg) noexcept { return addNoCpy(msg); }
    template<typename... Args> requires (sizeof...(Args) > 0u)
    default_exception& add(std::format_string<Args...> f_str, Args&&... types) noexcept {
        return this->addNoCpy(std::format(f_str, std::forward<Args>(types)...));
    }



    [[nodiscard]] std::string string() const noexcept {
        return std::format("{0} {1} {2} {3} {4}", _msg, _details, func_string(), loc_string(), trace_string());
    }
    [[nodiscard]] std::string brief() const noexcept { return std::format("{0} {1} {2}", _msg, func_string(), loc_string()); }

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
    default_exception& addNoCpy(std::string const& msg) noexcept {
        if(_details.empty()) _details = "DETAILS:\n";
        _details += std::format("\t{}\n", msg);

        _what = string();
        return *this;
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

    default_exception(default_exception const& other) noexcept = default;
    default_exception(default_exception&& other) noexcept = default;
    default_exception& operator=(default_exception const& other) noexcept = default;
    default_exception& operator=(default_exception&& other) noexcept = default;
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

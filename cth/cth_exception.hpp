#pragma once
// ReSharper disable CppClangTidyModernizePassByValue
//bc exceptions need to copy the data
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
constexpr static std::string_view to_string(const Severity sev) {
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
    explicit default_exception(const Severity severity, std::string msg, std::source_location loc = std::source_location::current(),
        std::stacktrace trace = std::stacktrace::current()) : _severity(severity), _logMsg{std::format(" {0}\n", msg)},
        _loc{loc}, _trace{std::move(trace)} {}


    void add(std::string msg) noexcept {
        addNoCpy(msg);
    }
    template<typename... Args> requires (sizeof...(Args) > 0u)
    void add(const std::format_string<Args...> f_str, Args&&... types) noexcept {
        this->addNoCpy(std::format(f_str, std::forward<Args>(types)...));
    }

    [[nodiscard]] Severity severity() const noexcept { return _severity; }
    [[nodiscard]] const char* what() const noexcept override { return _logMsg.c_str(); }
    [[nodiscard]] std::string details() const noexcept { return _detailsMsg; }

    [[nodiscard]] std::stacktrace stacktrace() const noexcept { return _trace; }
    [[nodiscard]] std::source_location location() const noexcept { return _loc; }

    [[nodiscard]] std::string string() const noexcept { return std::format("{0} {1} {2} {3} {4}", _logMsg, _detailsMsg, func_string(), loc_string(), trace_string()); }
    [[nodiscard]] std::string loc_string() const noexcept {
        const std::string filename = std::string(_loc.file_name());
        return std::format("LOCATION: {0}({1}:{2})\n", filename.substr(filename.find_last_of('\\') + 1),
            _loc.line(), _loc.column());
    }
    [[nodiscard]] std::string func_string() const noexcept {
        return std::format("FUNCTION: {0}\n", _loc.function_name());
    }
    [[nodiscard]] std::string trace_string() const noexcept {
        std::string str = "STACKTRACE:\n";

        std::for_each(_trace.begin(), _trace.end() - 2, [&](const auto& entry) {
            const std::filesystem::path path{entry.source_file()};
            str += std::format("\t{2} : {0}({1})\n", path.filename().string(), entry.source_line(), entry.description());
        });
        return str;
    }

private:
    void addNoCpy(const std::string& msg) noexcept {
        if(_detailsMsg.empty()) _detailsMsg = "DETAILS:\n";
        _detailsMsg += '\t';
        _detailsMsg += msg;
        _detailsMsg += '\n';
    }

    Severity _severity;
    std::string _logMsg;
    std::string _detailsMsg{};
    std::source_location _loc;
    std::stacktrace _trace;
};
template<typename T>
class data_exception : public default_exception {
public:
    data_exception(const Severity severity, std::string msg, T data, std::source_location loc = std::source_location::current(),
        std::stacktrace trace = std::stacktrace::current()) : default_exception{severity, std::move(msg), std::move(loc), std::move(trace)},
        _dataObj{std::move(data)} {}
    data_exception(T data, default_exception exception) : default_exception{std::move(exception)}, _dataObj{std::move(data)} {}
    [[nodiscard]] T data() const noexcept { return _dataObj; }
private:
    T _dataObj;
};


} // namespace cth::except

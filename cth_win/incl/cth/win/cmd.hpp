#pragma once

#include <filesystem>
#include <format>


namespace cth::win::cmd {
/**
 * runs a command hidden with `cmd /c` in a directory
 * @details blocks until the process ends
 * @param dir directory to run the command in
 * @param command to run
 * @return exit code of the process, `EXIT_FAILURE` -> the process was not created
 * @throws cth::except::win_exception reason: waiting for the process failed
 * @note paths in `command` only work with `'\'`, not with `'/'`
 */
[[nodiscard]] size_t hidden_dir(std::string_view dir, std::string_view command);

/**
 * runs a formatted command hidden with `cmd /c` in a directory
 * @details blocks until the process ends
 * @details formats the command with `std::format`
 * @param dir directory to run the command in
 * @param command format string of the command to run
 * @param types arguments of the format string
 * @return exit code of the process, `EXIT_FAILURE` -> the process was not created
 * @throws cth::except::win_exception reason: waiting for the process failed
 * @note paths in `command` only work with `'\'`, not with `'/'`
 */
template<typename... Types>
requires(sizeof...(Types) > 0u)
[[nodiscard]] size_t
hidden_dir(std::string_view dir, std::format_string<Types...> command, Types&&... types) {
    return cth::win::cmd::hidden_dir(dir, std::format(command, std::forward<Types>(types)...));
}

/**
 * runs a command hidden with `cmd /c` in the current directory
 * @details blocks until the process ends
 * @param command to run
 * @return exit code of the process, `EXIT_FAILURE` -> the process was not created
 * @throws cth::except::win_exception reason: waiting for the process failed
 * @note paths in `command` only work with `'\'`, not with `'/'`
 */
[[nodiscard]] inline size_t hidden(std::string_view command) {
    return cth::win::cmd::hidden_dir(std::filesystem::current_path().string(), command);
}

/**
 * runs a formatted command hidden with `cmd /c` in the current directory
 * @details blocks until the process ends
 * @details formats the command with `std::format`
 * @param command format string of the command to run
 * @param types arguments of the format string
 * @return exit code of the process, `EXIT_FAILURE` -> the process was not created
 * @throws cth::except::win_exception reason: waiting for the process failed
 * @note paths in `command` only work with `'\'`, not with `'/'`
 */
template<typename... Types>
requires(sizeof...(Types) > 0u)
[[nodiscard]] size_t hidden(std::format_string<Types...> command, Types&&... types) {
    return cth::win::cmd::hidden(std::format(command, std::forward<Types>(types)...));
}
}

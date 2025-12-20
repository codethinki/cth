#pragma once

#include <filesystem>
#include <format>


namespace cth::win::cmd {
/**
 * @brief executes a cmd command with location in the background
 * @param dir directory to execute the command in
 * @param command = "cmd.exe /c {command}"
 * @note paths only work with '\' NOT '/'
 */
[[nodiscard]] size_t hidden_dir(std::string_view dir, std::string_view command);

/**
 * @brief executes a cmd command with location in the background
 * @param command = "cmd.exe /c (...)"
 * @note paths only work with '\' NOT '/'
 */
template<typename... Types> requires(sizeof...(Types) > 0u)
[[nodiscard]] size_t hidden_dir(std::string_view dir, std::format_string<Types...> command, Types&&... types) {
    return cth::win::cmd::hidden_dir(dir, std::format(command, std::forward<Types>(types)...));
}

/**
 * @brief executes a cmd command with location in the background
 * @param command = "cmd.exe /c (...)"
 * @note paths only work with '\' NOT '/'
 */
[[nodiscard]] inline size_t hidden(std::string_view command) {
    return cth::win::cmd::hidden_dir(std::filesystem::current_path().string(), command);
}

/**
 * @brief executes a cmd command in the background
 * @param command already contains "cmd.exe /c"
 * @note command is executed in the proc dir
 * @note paths only work with '\' NOT '/'
 */
template<typename... Types> requires(sizeof...(Types) > 0u)
[[nodiscard]] size_t hidden(std::format_string<Types...> command, Types&&... types) {
    return cth::win::cmd::hidden(std::format(command, std::forward<Types>(types)...));
}
}

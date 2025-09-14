#pragma once
#include "cth/io/log.hpp"

#include <filesystem>



namespace cth::win {
using dword_t = unsigned long;



namespace cmd {
    /**
    * \brief executes a cmd command with location in the background
    * \param command = "cmd.exe /c {command}"
    * \note paths only work with '\' NOT '/'
    */
    [[nodiscard]] int hidden_dir(std::string_view dir, std::string_view command);
    /**
    * \brief executes a cmd command with location in the background
    * \param command = "cmd.exe /c (...)"
    * \note paths only work with '\' NOT '/'
    */
    template<typename... Types> requires (sizeof...(Types) > 0u)
    [[nodiscard]] int hidden_dir(std::string_view dir, std::format_string<Types...> command,
        Types&&... types) {
        return cth::win::cmd::hidden_dir(dir, std::format(command, std::forward<Types>(types)...));
    }

    /**
    * \brief executes a cmd command with location in the background
    * \param command = "cmd.exe /c (...)"
    * \note paths only work with '\' NOT '/'
    */
    [[nodiscard]] inline int hidden(std::string_view command) {
        return cth::win::cmd::hidden_dir(std::filesystem::current_path().string(), command);
    }

    /**
    * \brief executes a cmd command in the background
    * \param command already contains "cmd.exe /c"
    * \note command is executed in the proc dir
    * \note paths only work with '\' NOT '/'
    */
    template<typename... Types> requires(sizeof...(Types) > 0u)
    [[nodiscard]] int hidden(std::format_string<Types...> command, Types&&... types) {
        return cth::win::cmd::hidden(std::format(command, std::forward<Types>(types)...));
    }

} // namespace cmd


namespace clipbd {
    std::string getText();
} // namespace clipbd

namespace proc {

    /**
     * \brief returns elevation status
     * \return true := admin
     */
    bool elevated();

    /**
     * @brief enumerates all processes with id's
     * @return not @ref std::vector::empty() if successful
     */
    std::vector<dword_t> enumerate();

    std::optional<std::wstring> name(dword_t process_id, bool full_path = false);


    std::vector<dword_t> find(std::wstring_view process_name, bool full_path = false);

    /**
     * @brief checks if process_name is active
     * @return result if successful, else @ref std::nullopt
     */
    std::optional<bool> active(std::wstring_view process_name);



    /**
    * @brief counts the number of process instances
    * @return number of processes if successful, else @ref std::nullopt
    */
    std::optional<size_t> count(std::wstring_view process_name);

} // namespace proc

namespace desktop {
    void getResolution(uint32_t& horizontal, uint32_t& vertical);
}


namespace io {
    void readUnbuffered(std::string_view path, std::vector<char>& buffer);
}

}

#pragma once
#include"cth_concepts.hpp"

#include <filesystem>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>



namespace cth::win {



namespace cmd {
    /**
    * \brief executes a cmd command with location in the background
    * \param command = "cmd.exe /c (...)"
    * \note paths only work with '\' NOT '/'
    */
    [[nodiscard]] inline int hidden_dir(std::string_view dir, std::string_view command);
    /**
    * \brief executes a cmd command with location in the background
    * \param command = "cmd.exe /c (...)"
    * \note paths only work with '\' NOT '/'
    */
    template<typename... Types> requires (sizeof...(Types) > 0u)
    [[nodiscard]] int hidden_dir(std::string_view dir, std::format_string<Types...> command, Types&&... types) {
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
    inline bool elevated();

    inline bool active(std::wstring_view process_name);
    inline uint32_t instances(std::wstring_view process_name);
} // namespace proc

namespace desktop {
    inline void getResolution(uint32_t& horizontal, uint32_t& vertical);
}


namespace io {
    void readUnbuffered(std::string_view path, std::vector<char>& buffer);
}

} // namespace cth::win


#include "windows/cth_windows.inl"

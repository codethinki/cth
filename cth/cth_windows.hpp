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
    [[nodiscard]] int hidden_dir(const std::string_view dir, std::format_string<Types...> command, Types&&... types) {
        return cth::win::cmd::hidden_dir(dir, std::format(command, std::forward<Types>(types)...));
    }

    /**
    * \brief executes a cmd command with location in the background
    * \param command = "cmd.exe /c (...)"
    * \note paths only work with '\' NOT '/'
    */
    [[nodiscard]] inline int hidden(const std::string_view command) {
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
    inline bool elevated() {
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        PSID adminGroup = nullptr;
        BOOL isAdmin = AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup);

        if(isAdmin && !CheckTokenMembership(nullptr, adminGroup, &isAdmin)) isAdmin = false;

        FreeSid(adminGroup);

        return isAdmin;
    }

    inline bool active(std::wstring_view process_name);
    inline uint32_t instances(std::wstring_view process_name);
} // namespace proc

namespace desktop {
    inline void getResolution(uint32_t& horizontal, uint32_t& vertical) {
        RECT desktop;
        HWND hDesktop = GetDesktopWindow();
        GetWindowRect(hDesktop, &desktop);
        horizontal = desktop.right;
        vertical = desktop.bottom;
    }
}


namespace file {
    void readUnbuffered(std::string_view path, std::vector<char>& buffer);

    namespace dev {
        template<type::char_t T>
        std::vector<std::basic_string<T>> loadTxt(std::basic_string_view<T> path);
    }


    inline std::vector<std::string> loadTxt(const std::string_view path) { return dev::loadTxt<char>(path); }
    inline std::vector<std::wstring> loadTxt(const std::wstring_view path) { return dev::loadTxt<wchar_t>(path); }
}

} // namespace cth::win


#include "inl/cth_windows.inl"

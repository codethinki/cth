#pragma once

#include <fstream>
#include "../cth_log.hpp"

#include <TlHelp32.h>


namespace cth::win {
namespace cmd {

    inline int hidden_dir(const std::string_view dir, const std::string_view command) {
        PROCESS_INFORMATION pInfo{};
        STARTUPINFOA sInfo{};
        sInfo.cb = sizeof(sInfo);

        std::string cmd = std::format("cmd /c \"{}\"", command);

        const bool res = CreateProcessA(nullptr, cmd.data(),
            nullptr, nullptr, false, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
            nullptr, dir.data(), &sInfo, &pInfo);


        CTH_WARN(res != 1, "cmd process creation failed") {
            details->add("cmd: {}", cmd);
            details->add("location: {}", dir);
            return EXIT_FAILURE;
        }


        WaitForSingleObject(pInfo.hProcess, INFINITE);

        DWORD returnValue = 0;
        GetExitCodeProcess(pInfo.hProcess, &returnValue);

        CloseHandle(pInfo.hProcess);
        CloseHandle(pInfo.hThread);

        return static_cast<int>(returnValue);
    }

} // namespace cmd


namespace clipbd {
    inline std::string getText() {
        std::string text;

        CTH_STABLE_WARN(!OpenClipboard(nullptr), "getClipboardText: no clipboard access");

        const HANDLE hData = GetClipboardData(CF_TEXT);
        const char* pszText = static_cast<char*>(GlobalLock(hData));

        if(pszText != nullptr) text = pszText;

        GlobalUnlock(hData);
        CloseClipboard();
        return text;
    }
} // namespace clipbd


namespace proc {
    inline bool elevated() {
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        PSID adminGroup = nullptr;
        BOOL isAdmin = AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup);

        if(isAdmin && !CheckTokenMembership(nullptr, adminGroup, &isAdmin)) isAdmin = false;

        FreeSid(adminGroup);

        return isAdmin;
    }
    inline bool active(const std::wstring_view process_name) {
        PROCESSENTRY32 proc;
        proc.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        Process32First(snapshot, &proc);
        while(process_name != proc.szExeFile)
            Process32Next(snapshot, &proc);
        return process_name == proc.szExeFile;
    }
    inline uint32_t instances(const std::wstring_view process_name) {
        uint32_t processCount = 0;

        PROCESSENTRY32 proc;
        proc.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        Process32First(snapshot, &proc);
        do { if(process_name == proc.szExeFile) processCount += 1; } while(Process32Next(snapshot, &proc));

        return processCount;
    }
}

namespace desktop {
    void getResolution(uint32_t& horizontal, uint32_t& vertical) {
        RECT desktop;
        HWND hDesktop = GetDesktopWindow();
        GetWindowRect(hDesktop, &desktop);
        horizontal = desktop.right;
        vertical = desktop.bottom;
    }
}

namespace file {

    inline void readUnbuffered(const std::string_view path, std::vector<char>& buffer) {
        HANDLE handle = CreateFileA(path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);
        CTH_STABLE_ERR(handle == INVALID_HANDLE_VALUE, "failed to create handle for file ({})", path) throw details->exception();


        DWORD fileSize = GetFileSize(handle, nullptr);


        buffer.resize(fileSize + 4096ULL - fileSize % 4096ULL);

        auto event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        _OVERLAPPED overlapped{.hEvent = event};

        DWORD bytesRead = 0;
        ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, &overlapped);
        const BOOL result = GetOverlappedResult(handle, &overlapped, &bytesRead, TRUE);

        CTH_STABLE_ERR(!result, "failed to read file ({})", path) {
            details->add("file size: {}", fileSize);
            details->add("buffer size: {}", buffer.size());
            details->add("last error: {}", GetLastError());
            throw details->exception();
        }


        buffer.resize(fileSize);
    }
}

namespace file::dev {



    template<type::character T>
    std::vector<std::basic_string<T>> loadTxt(const std::basic_string_view<T> path) {
        std::basic_ifstream<T> file(path.data());
        if(std::is_same_v<T, wchar_t>) file.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
        std::basic_string<T> s;
        std::vector<std::basic_string<T>> strings;

        while(std::getline(file, s)) strings.push_back(s);

        return strings;
    }
}

} // namespace cth::win

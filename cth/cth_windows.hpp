#pragma once

#include "cth_log.hpp"
#include "cth_string.hpp"

#include <filesystem>
#include <Windows.h>
#include <TlHelp32.h>



namespace cth::win {
namespace cmd {
    using namespace std;
    /**
     * \brief executes a cmd command in the background
     * \param exec_dir (default: current dir)
     */
    inline int hidden(wstring command, const wstring& exec_dir = filesystem::current_path()) {
        PROCESS_INFORMATION pInfo{};
        STARTUPINFO sInfo{};
        sInfo.cb = sizeof(sInfo);

        command = L"cmd.exe /c" + command;
        const bool res = CreateProcessW(nullptr, const_cast<wchar_t*>(command.c_str()),
            nullptr, nullptr, false, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
            nullptr, exec_dir.c_str(), &sInfo, &pInfo);

        if(!res) return EXIT_FAILURE; //IMPLEMENT the planned cth_stable_warn here

        WaitForSingleObject(pInfo.hProcess, INFINITE);

        DWORD returnValue = 0;
        GetExitCodeProcess(pInfo.hProcess, &returnValue);

        CloseHandle(pInfo.hProcess);
        CloseHandle(pInfo.hThread);

        return static_cast<int>(returnValue);
    }
    /**
     * \brief executes a cmd command in the background
     * \param exec_dir (default: current dir)
     */
    inline int hidden(const string& command, const string& exec_dir = filesystem::current_path().string()) {
        return hidden(cth::str::conv::toW(command), str::conv::toW(exec_dir));
    }
}

namespace clipbd {
    inline string getText() {
        string text;

        CTH_STABLE_WARN(!OpenClipboard(nullptr) && "getClipboardText: no clipboard access");

        const HANDLE hData = GetClipboardData(CF_TEXT);
        const char* pszText = static_cast<char*>(GlobalLock(hData));

        if(pszText != nullptr) text = pszText;

        GlobalUnlock(hData);
        CloseClipboard();
        return text;
    }
} // namespace clipbd

namespace proc {
    inline bool is_elevated() {
        SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
        PSID adminGroup = nullptr;
        BOOL isAdmin = AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup);

        if(isAdmin && !CheckTokenMembership(nullptr, adminGroup, &isAdmin)) isAdmin = false;

        FreeSid(adminGroup);

        return isAdmin;
    }

    inline bool processActive(const std::wstring& process_name) {
        PROCESSENTRY32 proc;
        proc.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        Process32First(snapshot, &proc);
        while(process_name != proc.szExeFile)
            Process32Next(snapshot, &proc);
        return process_name == proc.szExeFile;
    }
    inline uint32_t processInstances(const wstring& process_name) {
        uint32_t processCount = 0;

        PROCESSENTRY32 proc;
        proc.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        Process32First(snapshot, &proc);
        do { if(process_name == proc.szExeFile) processCount += 1; } while(Process32Next(snapshot, &proc));

        return processCount;
    }
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



} // namespace cth::win

#pragma once
#include <Windows.h>
#include <TlHelp32.h>

namespace cth::windows {
namespace clipbd {
    inline string getText() {
        string text;

        assert(!OpenClipboard(nullptr) && "getClipboardText: cant open clipboard");

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



} // namespace cth::windows

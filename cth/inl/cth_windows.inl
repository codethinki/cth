#pragma once

#include <filesystem>
#include <fstream>
#include "../cth_log.hpp"

#include <TlHelp32.h>


namespace cth::win {
namespace cmd {
    inline int hidden_dir(const string_view dir, const string_view command) {
        PROCESS_INFORMATION pInfo{};
        STARTUPINFOA sInfo{};
        sInfo.cb = sizeof(sInfo);

        string cmd = std::format("cmd /c \"{}\"", command);

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

}


namespace clipbd {
    inline string getText() {
        string text;

        CTH_STABLE_WARN(!OpenClipboard(nullptr), "getClipboardText: no clipboard access");

        const HANDLE hData = GetClipboardData(CF_TEXT);
        const char* pszText = static_cast<char*>(GlobalLock(hData));

        if(pszText != nullptr) text = pszText;

        GlobalUnlock(hData);
        CloseClipboard();
        return text;
    }
}


namespace proc {
    inline bool active(const wstring_view process_name) {
        PROCESSENTRY32 proc;
        proc.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        Process32First(snapshot, &proc);
        while(process_name != proc.szExeFile)
            Process32Next(snapshot, &proc);
        return process_name == proc.szExeFile;
    }
    inline uint32_t instances(const wstring_view process_name) {
        uint32_t processCount = 0;

        PROCESSENTRY32 proc;
        proc.dwSize = sizeof(PROCESSENTRY32);
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        Process32First(snapshot, &proc);
        do { if(process_name == proc.szExeFile) processCount += 1; } while(Process32Next(snapshot, &proc));

        return processCount;
    }
}


namespace file::dev {


    template<type::char_t T>
    vector<basic_string<T>> loadTxt(const basic_string_view<T> path) {
        basic_ifstream<T> file(path.data());
        if(is_same_v<T, wchar_t>) file.imbue(locale(locale::empty(), new std::codecvt_utf8<wchar_t>));
        basic_string<T> s;
        vector<basic_string<T>> strings;

        while(std::getline(file, s)) strings.push_back(s);

        return strings;
    }
}

} // namespace cth::win

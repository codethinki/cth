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


namespace file {

    inline void readUnbuffered(const string_view path, vector<char>& buffer) {
        HANDLE handle = CreateFileA(path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);
        CTH_STABLE_ERR(handle == INVALID_HANDLE_VALUE, "failed to create handle for file ({})", path) throw details->exception();


        DWORD fileSize = GetFileSize(handle, nullptr);


        buffer.resize(fileSize + 4096ULL - fileSize % 4096ULL);

        auto event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

        _OVERLAPPED overlapped{.hEvent = event};

        DWORD bytesRead = 0;
        ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, &overlapped);

        CTH_STABLE_ERR(!GetOverlappedResult(handle, &overlapped, &bytesRead, TRUE), "failed to read file ({})", path) {
            details->add("file size: {}", fileSize);
            details->add("buffer size: {}", buffer.size());
            details->add("last error: {}", GetLastError());
            throw details->exception();
        }


        buffer.resize(fileSize);
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

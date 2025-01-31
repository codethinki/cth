#pragma once
#include "../../io/log.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include<Windows.h>

#include <Psapi.h>

#include <array>
#include <memory>


namespace cth::win::cmd {
inline int hidden_dir(std::string_view dir, std::string_view command) {
    PROCESS_INFORMATION pInfo{};
    STARTUPINFOA sInfo{};
    sInfo.cb = sizeof(sInfo);

    std::string cmd = std::format("cmd /c \"{}\"", command);

    bool const res = CreateProcessA(nullptr, cmd.data(),
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


namespace cth::win::clipbd {
inline std::string getText() {
    std::string text;

    CTH_STABLE_WARN(!OpenClipboard(nullptr), "getClipboardText: no clipboard access") {}

    HANDLE const hData = GetClipboardData(CF_TEXT);
    char const* pszText = static_cast<char*>(GlobalLock(hData));

    if(pszText != nullptr) text = pszText;

    GlobalUnlock(hData);
    CloseClipboard();
    return text;
}
} // namespace clipbd


namespace cth::win::proc {
inline bool elevated() {
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup = nullptr;
    BOOL isAdmin = AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup);

    if(isAdmin && !CheckTokenMembership(nullptr, adminGroup, &isAdmin)) isAdmin = false;

    FreeSid(adminGroup);

    return isAdmin;
}

inline auto enumerate() -> std::vector<DWORD> {
    std::vector<DWORD> processIds(1024);
    DWORD bytesReturned = 0;


    while(true) {
        auto const idsByteSize = static_cast<DWORD>(processIds.size() * sizeof(DWORD));
        if(EnumProcesses(processIds.data(), idsByteSize, &bytesReturned) == 0) return {};
        if(idsByteSize != bytesReturned) break;

        processIds.resize(processIds.size() * 2);
    }

    processIds.resize(bytesReturned / sizeof(DWORD));
    return processIds;
}

inline std::optional<std::wstring> name(DWORD process_id, bool full_path) {
    std::unique_ptr<void, void(*)(HANDLE)> const process{
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id),
        [](HANDLE ptr) { CloseHandle(ptr); }
    };
    if(process == nullptr) return std::nullopt;
    std::array<wchar_t, MAX_PATH> processName{};
    DWORD nameSize = MAX_PATH;

    auto const success = QueryFullProcessImageNameW(process.get(), 0, processName.data(), &nameSize) != 0;

    if(!success) return std::nullopt;

    auto const end = processName.begin() + nameSize;

    if(full_path) return std::wstring{processName.begin(), end};

    auto const begin = std::ranges::next(std::ranges::find_last(processName, L'\\').begin());
    return std::wstring{std::from_range, std::ranges::subrange{begin, end}};
}

inline std::optional<bool> active(std::wstring_view process_name) {
    auto const& processIds = enumerate();

    for(auto const id : processIds)
        if(auto const str = name(id, false); str.has_value() && str == process_name) 
            return true;


    return false;
}

inline std::optional<size_t> instances(std::wstring_view process_name) {
    auto const& processIds = enumerate();

    size_t instanceCount = 0;

    for(auto const id : processIds)
        if(auto const str = name(id, false); str.has_value() && str == process_name)
            ++instanceCount;


    return instanceCount;
}
}

namespace cth::win::desktop {
void getResolution(uint32_t& horizontal, uint32_t& vertical) {
    RECT desktop;
    HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}
}

namespace cth::win::io {

inline void readUnbuffered(std::string_view path, std::vector<char>& buffer) {
    HANDLE handle = CreateFileA(path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);
    CTH_STABLE_ERR(handle == INVALID_HANDLE_VALUE, "failed to create handle for io ({})", path) throw details->exception();


    DWORD fileSize = GetFileSize(handle, nullptr);


    buffer.resize(fileSize + 4096ULL - fileSize % 4096ULL);

    auto event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    _OVERLAPPED overlapped{.hEvent = event};

    DWORD bytesRead = 0;
    ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, &overlapped);
    BOOL const result = GetOverlappedResult(handle, &overlapped, &bytesRead, TRUE);

    CTH_STABLE_ERR(!result, "failed to read io ({})", path) {
        details->add("io size: {}", fileSize);
        details->add("buffer size: {}", buffer.size());
        details->add("last error: {}", GetLastError());
        throw details->exception();
    }


    buffer.resize(fileSize);
}
}

#include "cth/win/windows.hpp"

#include "cth/io/log.hpp"

#include "cth/win/win_include.hpp"


#include <array>
#include <memory>

namespace {

template<class... Args>
void stable_assert(bool result, std::format_string<Args...> message, Args&&... args) {
    CTH_STABLE_ERR(!result, "windows function call failed") {
        details->add(message, std::forward<Args>(args)...);
        details->add("err code: {}", GetLastError());
        throw details->exception();
    }
}
}

namespace cth::win {
static_assert(sizeof(DWORD) == sizeof(dword_t), "weird DWORD size, report");
}

namespace cth::win::cmd {
int hidden_dir(std::string_view dir, std::string_view command) {
    PROCESS_INFORMATION pInfo{};
    STARTUPINFOA sInfo{};
    sInfo.cb = sizeof(sInfo);

    auto cmd = std::format("cmd /c \"{}\"", command);

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
std::string getText() {
    std::string text;

    CTH_STABLE_WARN(!OpenClipboard(nullptr), "getClipboardText: no clipboard access") {}

    auto const hData = GetClipboardData(CF_TEXT);
    char const* pszText = static_cast<char*>(GlobalLock(hData));

    if(pszText != nullptr) text = pszText;

    GlobalUnlock(hData);
    CloseClipboard();
    return text;
}
} // namespace clipbd


namespace cth::win::proc {
bool elevated() {
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup = nullptr;
    BOOL isAdmin = AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup);

    if(isAdmin && !CheckTokenMembership(nullptr, adminGroup, &isAdmin)) isAdmin = false;

    FreeSid(adminGroup);

    return isAdmin;
}

auto enumerate() -> std::vector<dword_t> {
    std::vector<DWORD> processIds(16);
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

std::optional<std::wstring> name([[maybe_unused]] dword_t process_id, bool full_path) {
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

std::vector<dword_t> find(std::wstring_view process_name, bool full_path) {
    auto const& ids = enumerate();

    std::vector<DWORD> matches{};
    for(auto const id : ids)
        if(process_name == name(id, full_path))
            matches.emplace_back(id);

    return matches;
}

std::optional<bool> active(std::wstring_view process_name) {
    auto const& processIds = enumerate();

    for(auto const id : processIds)
        if(auto const str = name(id, false); str.has_value() && str == process_name)
            return true;


    return false;
}



std::optional<size_t> count(std::wstring_view process_name) {
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

void readUnbuffered(std::string_view path, std::vector<char>& buffer) {
    cxpr static size_t PAGE_SIZE = 4096;

    auto handle = CreateFileA(path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
        FILE_FLAG_NO_BUFFERING, nullptr);

    CTH_STABLE_ERR(handle == INVALID_HANDLE_VALUE, "failed to create handle for io ({})", path)
        throw details->exception();


    DWORD fileSize = GetFileSize(handle, nullptr);


    buffer.resize(fileSize + PAGE_SIZE - fileSize % PAGE_SIZE);

    auto const event = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    _OVERLAPPED overlapped{.hEvent = event};

    DWORD bytesRead = 0;
    auto const readFileResult = ReadFile(handle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead,
        &overlapped);
    stable_assert(readFileResult, "failed to read file: {}", path);

    auto const queryResult = GetOverlappedResult(handle, &overlapped, &bytesRead, TRUE);

    stable_assert(queryResult, "failed to query overlapped result, file size: {}, buffer size: {}", fileSize,
        buffer.size());

    buffer.resize(fileSize);
}
}

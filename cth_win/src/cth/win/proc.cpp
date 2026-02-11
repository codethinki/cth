#include "cth/win/proc.hpp"

#include "cth/win/string.hpp"
#include "cth/win/win_types.hpp"
#include "win_include.hpp"
#include <cth/io/log.hpp>


#include <array>
#include <memory>


namespace cth::win {
static_assert(sizeof(DWORD) == sizeof(dword_t), "weird DWORD size, report");
}


namespace cth::win::proc {
bool elevated() {
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID adminGroup = nullptr;
    BOOL isAdmin = AllocateAndInitializeSid(
        &ntAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0,
        0,
        0,
        0,
        0,
        0,
        &adminGroup
    );

    if(isAdmin && !CheckTokenMembership(nullptr, adminGroup, &isAdmin))
        isAdmin = false;

    FreeSid(adminGroup);

    return isAdmin;
}

auto enumerate() -> std::vector<dword_t> {
    std::vector<DWORD> processIds(16);
    DWORD bytesReturned = 0;


    while(true) {
        auto const idsByteSize = static_cast<DWORD>(processIds.size() * sizeof(DWORD));
        if(EnumProcesses(processIds.data(), idsByteSize, &bytesReturned) == 0)
            return {};

        if(idsByteSize != bytesReturned)
            break;

        processIds.resize(processIds.size() * 2);
    }

    processIds.resize(bytesReturned / sizeof(DWORD));
    return processIds;
}

std::optional<std::wstring> name([[maybe_unused]] dword_t process_id, bool full_path) {
    std::unique_ptr<void, void (*)(HANDLE)> const process{
        OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id),
        [](HANDLE ptr) { CloseHandle(ptr); }
    };
    if(process == nullptr)
        return std::nullopt;
    std::array<wchar_t, MAX_PATH> processName{};
    DWORD nameSize = MAX_PATH;

    auto const success = QueryFullProcessImageNameW(process.get(), 0, processName.data(), &nameSize) != 0;

    if(!success)
        return std::nullopt;

    auto const end = processName.begin() + nameSize;

    if(full_path)
        return std::wstring{processName.begin(), end};

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
} // namespace cth::win::proc


namespace cth::win::io {} // namespace cth::win::io

#pragma once
#include "win_types.hpp"

#include <filesystem>

namespace cth::win {
namespace proc {

    /**
     * \brief returns elevation status
     * \return true if admin
     */
    bool elevated();

    /**
     * @brief enumerates all processes with id's
     * @return not @ref std::vector::empty() if successful
     */
    std::vector<dword_t> enumerate();

    std::optional<std::wstring> name(dword_t process_id, bool full_path = false);


    std::vector<dword_t> find(std::wstring_view process_name, bool full_path = false);

    /**
     * @brief checks if process_name is active
     * @return result if successful, else @ref std::nullopt
     */
    std::optional<bool> active(std::wstring_view process_name);


    /**
     * @brief counts the number of process instances
     * @return number of processes if successful, else @ref std::nullopt
     */
    std::optional<size_t> count(std::wstring_view process_name);

} // namespace proc

}

#pragma once
#include "win_types.hpp"

#include <filesystem>
#include <vector>

namespace cth::win::proc {

/**
 * checks if the process has admin rights
 * @return `true` if elevated
 */
bool elevated();

/**
 * lists the ids of all processes
 * @return process ids, empty -> enumerating failed
 */
std::vector<dword_t> enumerate();

std::optional<std::wstring> name(dword_t process_id, bool full_path = false);


std::vector<dword_t> find(std::wstring_view process_name, bool full_path = false);

/**
 * checks if a process with the name runs
 * @details never returns `std::nullopt`
 * @param process_name file name of the executable, without path
 * @return `true` if a process with the name runs, `false` -> none or `enumerate()` failed
 */
std::optional<bool> active(std::wstring_view process_name);


/**
 * counts the processes with the name
 * @details never returns `std::nullopt`
 * @param process_name file name of the executable, without path
 * @return number of processes, `0` -> none or `enumerate()` failed
 */
std::optional<size_t> count(std::wstring_view process_name);

}

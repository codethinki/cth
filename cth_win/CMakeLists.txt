glob_cpp(CTH_WIN_SRC "src")
glob_cpp(CTH_WIN_INCL "incl")

add_library(cth_win STATIC ${CTH_WIN_SRC} ${CTH_WIN_INCL})
add_library(cth::win ALIAS cth_win)

package_target_include_directories(
        cth_win
        PUBLIC incl
)

target_link_libraries(cth_win PUBLIC cth::cth)

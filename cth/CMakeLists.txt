glob_cpp(CTH_SRC "src")
glob_cpp(CTH_INCL "incl")

add_library(cth INTERFACE)
add_library(cth::cth ALIAS cth)

package_target_include_directories(
        cth
        INTERFACE incl
)

glob_cpp(PROFILE_SRC "src")

add_executable(profile ${PROFILE_SRC})

target_include_directories(
        profile
        PRIVATE src
)

target_link_libraries(profile PRIVATE cth::cth)

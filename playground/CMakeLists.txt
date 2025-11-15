glob_cpp(PLAYGROUND_SRC "src")

add_executable(playground ${PLAYGROUND_SRC})

target_include_directories(
        playground
        PRIVATE src
)

target_link_libraries(playground PRIVATE cth::cth)

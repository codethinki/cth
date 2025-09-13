
#[[[
# installable_include_directories
# calls target_include_directories, makes PUBLIC and INTERFACE headers installable
# and the export set is implicitly "${PROJECT_NAME}-targets".
#]]
function(installable_include_directories TARGET_NAME)
    # --- Setup ---
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs PUBLIC PRIVATE INTERFACE)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    include(GNUInstallDirs)
    set(EXPORT_SET_NAME "${PROJECT_NAME}-targets")

    # --- Part 1: Configure Include Directories ---
    foreach (SCOPE PUBLIC PRIVATE INTERFACE)
        if (DEFINED ARGS_${SCOPE})
            set(PROCESSED_DIRS "")
            foreach (DIR ${ARGS_${SCOPE}})
                list(APPEND PROCESSED_DIRS "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${DIR}>")
                if (NOT "${SCOPE}" STREQUAL "PRIVATE")
                    list(APPEND PROCESSED_DIRS "$<INSTALL_INTERFACE:${DIR}>")
                endif ()
            endforeach ()
            target_include_directories(${TARGET_NAME} ${SCOPE} ${PROCESSED_DIRS})
        endif ()
    endforeach ()

    # --- Part 2: Install Target and Public Headers ---
    install(TARGETS ${TARGET_NAME} EXPORT ${EXPORT_SET_NAME}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    set(PUBLIC_HEADER_DIRS ${ARGS_PUBLIC} ${ARGS_INTERFACE})
    if (PUBLIC_HEADER_DIRS)
        list(REMOVE_DUPLICATES PUBLIC_HEADER_DIRS)
        foreach (DIR ${PUBLIC_HEADER_DIRS})
            install(DIRECTORY ${DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
        endforeach ()
    endif ()
endfunction()


#[[[
# setup_install
#
# Creates cmake package configuration files for the project.
# EXPORT_SET = "${PROJECT_NAME}-targets" and NAMESPACE = "${PROJECT_NAME}::"
#]]

function(setup_install)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    # --- Use the project-wide convention for all names ---
    set(EXPORT_SET_NAME "${PROJECT_NAME}-targets")
    set(NAMESPACE "${PROJECT_NAME}::")
    set(INSTALL_CONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
    set(TARGETS_FILENAME "${EXPORT_SET_NAME}.cmake")

    # --- Part 1: Install the Export Set ---
    install(EXPORT ${EXPORT_SET_NAME}
            FILE ${TARGETS_FILENAME}
            NAMESPACE ${NAMESPACE}
            DESTINATION ${INSTALL_CONFIG_DIR}
    )

    # --- Part 2: Auto-generate and Install Config/Version files ---
    set(TEMP_CONFIG_IN_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake.in")
    file(WRITE ${TEMP_CONFIG_IN_PATH}
            "@PACKAGE_INIT@\n\n"
            "include(\"\${CMAKE_CURRENT_LIST_DIR}/${TARGETS_FILENAME}\")\n"
    )

    configure_package_config_file(${TEMP_CONFIG_IN_PATH}
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            INSTALL_DESTINATION ${INSTALL_CONFIG_DIR}
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
            DESTINATION ${INSTALL_CONFIG_DIR}
    )
endfunction()
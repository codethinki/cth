
#[[[
# package_target_include_directories(<target_name> [PUBLIC|PRIVATE|INTERFACE] <dirs>...)
# registers the target as package component
# calls target_include_directories, makes PUBLIC and INTERFACE headers part of the package
# if present, "${PROJECT_NAME}_" prefix is stripped from the target name for the EXPORT_NAME property
#
# The EXPORT_SET is named "${PROJECT_NAME}-targets".
#]]
function(package_target_include_directories TARGET_NAME)
    # --- setup ---
    set(options "")
    set(oneValueArgs "")
    set(multiValueArgs PUBLIC PRIVATE INTERFACE)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    include(GNUInstallDirs)
    set(EXPORT_SET_NAME "${PROJECT_NAME}-targets")

    # --- strip project name prefix for EXPORT_NAME ---
    set(PREFIX_TO_STRIP "${PROJECT_NAME}_")

    string(FIND "${TARGET_NAME}" "${PREFIX_TO_STRIP}" PREFIX_POS)

    if(PREFIX_POS EQUAL 0)
        string(LENGTH "${PREFIX_TO_STRIP}" PREFIX_LENGTH)
        string(SUBSTRING "${TARGET_NAME}" ${PREFIX_LENGTH} -1 CLEAN_EXPORT_NAME)
        set_property(TARGET ${TARGET_NAME} PROPERTY EXPORT_NAME ${CLEAN_EXPORT_NAME})
    endif()

    # --- configure include directories ---
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

    # --- install target and public headers ---
    install(TARGETS ${TARGET_NAME} EXPORT ${EXPORT_SET_NAME}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    set(PUBLIC_HEADER_DIRS ${ARGS_PUBLIC} ${ARGS_INTERFACE})
    if(PUBLIC_HEADER_DIRS)
        list(REMOVE_DUPLICATES PUBLIC_HEADER_DIRS)
        foreach(DIR ${PUBLIC_HEADER_DIRS})
            install(DIRECTORY ${DIR}/ DESTINATION ${DIR})
        endforeach()
    endif()
    # --- Register Target as Installable ---
    get_property(INSTALLABLE_TARGETS GLOBAL PROPERTY _PROJECT_INSTALLABLE_TARGETS)
    list(APPEND INSTALLABLE_TARGETS ${TARGET_NAME})
    set_property(GLOBAL PROPERTY _PROJECT_INSTALLABLE_TARGETS ${INSTALLABLE_TARGETS})
endfunction()

#[[[
# add_package_target()
# builds and installs all registered package targets
# creates a custom target named "${PROJECT_NAME}_install"
#]]
function(add_package_target)
    get_property(INSTALLABLE_TARGETS GLOBAL PROPERTY _PROJECT_INSTALLABLE_TARGETS)
    if (NOT INSTALLABLE_TARGETS)
        message(FATAL_ERROR "No installable targets were registered, use package_target_include_directories
        or add to INSTALLABLE_TARGETS manually")
    endif ()

    set(INSTALL_TARGET_NAME "${PROJECT_NAME}_package")
    set(INSTALL_COMMENT "Packaging ${PROJECT_NAME} project...")

    add_custom_target(${INSTALL_TARGET_NAME}
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_INSTALL_PREFIX}"
            COMMAND ${CMAKE_COMMAND} --install . --prefix "${CMAKE_INSTALL_PREFIX}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${INSTALLABLE_TARGETS}
            COMMENT "${INSTALL_COMMENT}"
    )
endfunction()


#[[[
# setup_package()
#
# Creates cmake package configuration files for the project.
# EXPORT_SET = "${PROJECT_NAME}-targets" and NAMESPACE = "${PROJECT_NAME}::"
#]]
function(setup_package)
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

#[[[
# packages the project by setting up the package
#]]
function(create_package)
    setup_package()
    add_package_target()
endfunction()


# cmake/WinDeploy.cmake
function(deploy_windows_dependencies target)
    # Optional flag to run windeployqt
    set(options USES_QT)
    cmake_parse_arguments(ARGS "${options}" "" "" ${ARGN})

    if(NOT WIN32)
        return()
    endif()

    if(ARGS_USES_QT)
        # Find windeployqt executable from the vcpkg installation
        get_filename_component(VCPKG_SCRIPT_DIR "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
        get_filename_component(VCPKG_ROOT "${VCPKG_SCRIPT_DIR}/../.." REALPATH)

        find_program(
            WINDEPLOYQT_EXECUTABLE windeployqt
            HINTS "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/tools/Qt6/bin"
                  "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/tools/qtbase/bin"
            NO_DEFAULT_PATH
        )

        if(NOT WINDEPLOYQT_EXECUTABLE)
            message(WARNING "Failed to find windeployqt.exe. Qt DLLs may not be deployed.")
        else()
            message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
            
            # Select --debug or --release flag based on configuration
            set(CONFIG_FLAG "$<IF:$<CONFIG:Debug>,--debug,--release>")

            # Use windeployqt to deploy Qt dependencies
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    ${CONFIG_FLAG}
                    --no-translations
                    --no-system-d3d-compiler
                    --no-opengl-sw
                    "$<TARGET_FILE:${target}>"
                COMMENT "Deploying Qt dependencies for ${target}"
            )
        endif()
    endif()

    # Deploy vcpkg DLLs - use TARGET_RUNTIME_DLLS for automatic dependency discovery
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying vcpkg runtime dependencies..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_RUNTIME_DLLS:${target}>"
            "$<TARGET_FILE_DIR:${target}>"
        COMMAND_EXPAND_LISTS
        COMMENT "Deploying vcpkg dependencies for ${target}"
    )

    # Install all DLLs from the build directory to CPack destination
    install(
        DIRECTORY "$<TARGET_FILE_DIR:${target}>/"
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        FILES_MATCHING PATTERN "*.dll"
    )

    # Also explicitly install the target executable itself
    install(TARGETS ${target}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT Runtime
    )
endfunction()

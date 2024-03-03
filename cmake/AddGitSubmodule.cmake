function(add_git_submodule dir)
    find_package(Git REQUIRED)

    # Check if the submodule exists
    if (NOT EXISTS ${dir})
        message(STATUS "Submodule '${dir}' not found. Cloning it...")
        execute_process(
            COMMAND ${GIT_EXECUTABLE}
            submodule update --init --recursive -- ${dir}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        )
    endif()

    # Check if the submodule comes with a CMakeLists.txt file
    if (EXISTS ${dir}/CMakeLists.txt)
        message(STATUS "Adding submodule '${dir}'")
        add_subdirectory(${dir})
    else()
        message(FATAL_ERROR "Submodule '${dir}' does not contain a CMakeLists.txt file")
    endif()

endfunction(add_git_submodule)
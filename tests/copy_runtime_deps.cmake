# Copy the exact runtime DLLs FO_TEST_EXE needs beside it, so gtest
# discovery can execute it at build time. Resolving from the linked
# executable (instead of globbing prefix bin dirs) avoids shadowing a
# good DLL with a same-named debug/other-build variant.
# -D inputs: FO_TEST_EXE, FO_TEST_DEST, FO_TEST_SEARCH_DIRS (; list).

file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${FO_TEST_EXE}"
    RESOLVED_DEPENDENCIES_VAR _resolved
    UNRESOLVED_DEPENDENCIES_VAR _unresolved
    DIRECTORIES ${FO_TEST_SEARCH_DIRS}
    PRE_EXCLUDE_REGEXES "^api-ms-" "^ext-ms-")

foreach(_dep IN LISTS _resolved)
    execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_dep}" "${FO_TEST_DEST}")
endforeach()

if(_unresolved)
    message(WARNING "fo_tests unresolved runtime dependencies: ${_unresolved}")
endif()

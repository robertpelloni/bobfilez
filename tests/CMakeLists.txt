# Tracked C++ test tree (restored 2026-11-17). The root CMake only adds
# this directory when tests/CMakeLists.txt exists, which is why ctest
# previously found no tests in fresh checkouts.
find_package(GTest CONFIG QUIET)
if(NOT GTest_FOUND)
    message(WARNING "GTest not found - fo_tests will not be built. "
                    "Install gtest (vcpkg manifest provides it) or pass its prefix via CMAKE_PREFIX_PATH.")
    return()
endif()

add_executable(fo_tests
    test_audit_logger.cpp
    test_cli_integration.cpp
    test_operation_repository.cpp
    test_provider_registry.cpp
    test_scanners.cpp
)
target_link_libraries(fo_tests PRIVATE fo_core GTest::gtest_main)

# fo_core links many shared provider libraries; gtest discovery executes
# fo_tests at build time, so the runtime DLLs must sit beside it first.
add_custom_command(TARGET fo_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:fo_tests> $<TARGET_FILE_DIR:fo_tests>
    COMMAND_EXPAND_LISTS)

# TARGET_RUNTIME_DLLS only covers imported targets; providers linked by
# plain library names (OpenCV's ${OpenCV_LIBS}, lean-prefix staged libs)
# contribute no DLL paths, and gtest discovery then fails with 0xc0000135
# in trees that lack them (observed in build-msvc). Copying whole prefix
# bin dirs instead shadowed good DLLs with debug variants (0xc0000005),
# so resolve the exact set the loader needs from the linked executable.
set(_fo_test_search_dirs "")
foreach(_prefix IN LISTS CMAKE_PREFIX_PATH)
    if(EXISTS "${_prefix}/bin")
        list(APPEND _fo_test_search_dirs "${_prefix}/bin")
    endif()
endforeach()
if(OpenCV_DIR AND EXISTS "${OpenCV_DIR}/../bin")
    list(APPEND _fo_test_search_dirs "${OpenCV_DIR}/../bin")
endif()
add_custom_command(TARGET fo_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND}
            "-DFO_TEST_EXE=$<TARGET_FILE:fo_tests>"
            "-DFO_TEST_DEST=$<TARGET_FILE_DIR:fo_tests>"
            "-DFO_TEST_SEARCH_DIRS=${_fo_test_search_dirs}"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/copy_runtime_deps.cmake)

include(GoogleTest)
gtest_discover_tests(fo_tests)

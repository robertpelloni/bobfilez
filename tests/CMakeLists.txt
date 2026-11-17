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
    test_operation_repository.cpp
    test_provider_registry.cpp
)
target_link_libraries(fo_tests PRIVATE fo_core GTest::gtest_main)

# fo_core links many shared provider libraries; gtest discovery executes
# fo_tests at build time, so the runtime DLLs must sit beside it first.
add_custom_command(TARGET fo_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:fo_tests> $<TARGET_FILE_DIR:fo_tests>
    COMMAND_EXPAND_LISTS)

include(GoogleTest)
gtest_discover_tests(fo_tests)

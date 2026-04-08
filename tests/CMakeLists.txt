enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(fo_tests
    test_rule_engine.cpp
    test_export.cpp
    test_hasher.cpp
    test_scanner.cpp
    test_database.cpp
    test_integration.cpp
    test_linter.cpp
    test_c_api.cpp
    test_search_engine.cpp
    test_file_watcher.cpp
    test_batch_rename.cpp
    test_audit_logger.cpp
    test_repositories.cpp
    test_treemap_pruner.cpp
    test_session_healing_hierarchy.cpp
    test_shadow_sorter_copy.cpp
    test_operation_repository.cpp
    test_vault_manager.cpp
    test_nexus_cleanup.cpp
    test_conversion_clip_dedup.cpp
)

target_link_libraries(fo_tests PRIVATE GTest::gtest GTest::gtest_main fo_core fo_c_api)

add_executable(fo_c_api_smoke
    c_api_smoke.c
)

target_link_libraries(fo_c_api_smoke PRIVATE fo_c_api)
set_target_properties(fo_c_api_smoke PROPERTIES LINKER_LANGUAGE CXX)

include(GoogleTest)
gtest_discover_tests(fo_tests)
add_test(NAME fo_c_api_smoke COMMAND fo_c_api_smoke)

# Fuzzing target (conditional)
option(FO_BUILD_FUZZ_TESTS "Build fuzz tests" OFF)
if(FO_BUILD_FUZZ_TESTS)
    add_executable(fuzz_metadata fuzz/fuzz_metadata.cpp)
    target_link_libraries(fuzz_metadata PRIVATE fo_core)
    # Note: MSVC doesn't support -fsanitize=fuzzer directly in the same way Clang does.
    # On Windows, we might need LibFuzzer linked manually or use Clang-CL.
    # For now, we will just define the target and let the user configure the compiler.
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fuzz_metadata PRIVATE -fsanitize=fuzzer)
        target_link_options(fuzz_metadata PRIVATE -fsanitize=fuzzer)
    endif()

    add_executable(fuzz_rule_engine fuzz/fuzz_rule_engine.cpp)
    target_link_libraries(fuzz_rule_engine PRIVATE fo_core)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(fuzz_rule_engine PRIVATE -fsanitize=fuzzer)
        target_link_options(fuzz_rule_engine PRIVATE -fsanitize=fuzzer)
    endif()
endif()

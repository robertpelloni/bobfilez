add_executable(fo_bench_dhash bench_dhash.cpp)

target_link_libraries(fo_bench_dhash PRIVATE fo_core)

target_include_directories(fo_bench_dhash PRIVATE ${CMAKE_SOURCE_DIR}/core/include)

target_compile_features(fo_bench_dhash PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(fo_bench_dhash PRIVATE /W4 /permissive-)
else()
  target_compile_options(fo_bench_dhash PRIVATE -Wall -Wextra -Wpedantic)
endif()

find_package(benchmark CONFIG QUIET)
if(benchmark_FOUND)
    add_executable(fo_benchmarks fo_benchmarks.cpp)
    target_link_libraries(fo_benchmarks PRIVATE fo_core benchmark::benchmark benchmark::benchmark_main)
    target_compile_features(fo_benchmarks PRIVATE cxx_std_20)
endif()

# Benchmark executables link fo_core's shared providers but are never
# executed at build time, so missing runtime DLLs (0xc0000135) went
# unnoticed. Copy the exact loader set beside them, like fo_tests.
if(WIN32)
    set(_fo_bench_search_dirs "")
    foreach(_prefix IN LISTS CMAKE_PREFIX_PATH)
        if(EXISTS "${_prefix}/bin")
            list(APPEND _fo_bench_search_dirs "${_prefix}/bin")
        endif()
    endforeach()
    if(OpenCV_DIR AND EXISTS "${OpenCV_DIR}/../bin")
        list(APPEND _fo_bench_search_dirs "${OpenCV_DIR}/../bin")
    endif()
    foreach(_bench_target fo_bench_dhash fo_benchmarks)
        if(TARGET ${_bench_target})
            add_custom_command(TARGET ${_bench_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_RUNTIME_DLLS:${_bench_target}> $<TARGET_FILE_DIR:${_bench_target}>
                COMMAND_EXPAND_LISTS)
            add_custom_command(TARGET ${_bench_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND}
                        "-DFO_TEST_EXE=$<TARGET_FILE:${_bench_target}>"
                        "-DFO_TEST_DEST=$<TARGET_FILE_DIR:${_bench_target}>"
                        "-DFO_TEST_SEARCH_DIRS=${_fo_bench_search_dirs}"
                        -P ${CMAKE_SOURCE_DIR}/tests/copy_runtime_deps.cmake
                DEPENDS ${CMAKE_SOURCE_DIR}/tests/copy_runtime_deps.cmake)
        endif()
    endforeach()
endif()


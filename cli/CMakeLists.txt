add_executable(fo_cli fo_cli.cpp cmd_lint.cpp)

target_link_libraries(fo_cli PRIVATE
    fo_core
)
if(WIN32)
    # CommandLineToArgvW for the UTF-8 argv bootstrap in main().
    target_link_libraries(fo_cli PRIVATE shell32)
endif()

target_compile_features(fo_cli PRIVATE cxx_std_20)

if(MSVC)
  target_compile_options(fo_cli PRIVATE /W4 /permissive-)
else()
  target_compile_options(fo_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

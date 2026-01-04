cmake_minimum_required(VERSION 3.16)

project(filezEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

option(FO_BUILD_CLI "Build the CLI engine" ON)
option(FO_BUILD_GUI "Build the Qt GUI" OFF)
option(FO_BUILD_BENCH "Build benchmark tools" ON)
option(FO_BUILD_TESTS "Build unit tests" ON)

# Read version from VERSION.md
file(READ "${CMAKE_SOURCE_DIR}/VERSION.md" FO_VERSION_STRING)
string(STRIP "${FO_VERSION_STRING}" FO_VERSION_STRING)

add_subdirectory(core)
if(FO_BUILD_CLI)
  add_subdirectory(cli)
endif()
if(FO_BUILD_GUI)
  add_subdirectory(gui)
endif()
if(FO_BUILD_BENCH)
  add_subdirectory(benchmarks)
endif()
if(FO_BUILD_TESTS)
  add_subdirectory(tests)
endif()

# --- CPack Configuration ---
if(FO_BUILD_CLI)
    # Set package information
    set(CPACK_PACKAGE_NAME "filez")
    set(CPACK_PACKAGE_VENDOR "filez Team")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-performance file organization tool")
    set(CPACK_PACKAGE_VERSION "${FO_VERSION_STRING}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "filez")
    set(CPACK_PACKAGE_CONTACT "maintainer@example.com")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

    # Install main executable and docs
    install(TARGETS fo_cli DESTINATION bin)
    install(FILES "${CMAKE_SOURCE_DIR}/README.md" "${CMAKE_SOURCE_DIR}/LICENSE.txt" DESTINATION .)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/docs/" DESTINATION docs)

    # MSI (WIX) specific settings
    set(CPACK_GENERATOR "WIX;ZIP")
    set(CPACK_WIX_UPGRADE_GUID "53483952-4950-454C-454F-5247414E495A") # Fixed GUID for upgrades
    # set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/docs/images/logo.ico") # Placeholder if needed
    # set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/docs/images/banner.bmp") # Placeholder
    # set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/docs/images/dialog.bmp") # Placeholder

    # Allow CMake to find WiX if installed via standard paths
    # Note: User must have WiX Toolset installed.

    include(CPack)
endif()

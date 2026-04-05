cmake_minimum_required(VERSION 3.16)
project(fo_omni LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick)

set(BOBUI_OMNICORE_DIR "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/omnicore")
file(GLOB_RECURSE BOBUI_OMNICORE_SOURCES CONFIGURE_DEPENDS
    "${BOBUI_OMNICORE_DIR}/src/*.cpp"
)

message(STATUS "fo_omni will use BobUI-provided Qt6 packages when available via CMAKE_PREFIX_PATH / BOBUI_ROOT")

add_executable(fo_omni
    src/main.cpp
    src/NativeUiBootstrap.cpp
    src/NativeUiBootstrap.hpp
    src/NativeUiLaunchProfile.cpp
    src/NativeUiLaunchProfile.hpp
    src/NativeUiProfileRegistry.cpp
    src/NativeUiProfileRegistry.hpp
    src/NativeUiRuntime.cpp
    src/NativeUiRuntime.hpp
    src/NativeUiRuntimeBundle.cpp
    src/NativeUiRuntimeBundle.hpp
    src/OmniQmlRegistration.cpp
    src/OmniQmlRegistration.hpp
    src/FileModel.cpp
    src/TreemapModel.cpp
    src/NativeMarkdownView.cpp
    assets/qml.qrc
    ${BOBUI_OMNICORE_SOURCES}
)

target_include_directories(fo_omni PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    "${BOBUI_OMNICORE_DIR}/include"
    "${BOBUI_OMNICORE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/juce"
    "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/imgui"
)

target_link_libraries(fo_omni PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    fo_core
)

install(TARGETS fo_omni DESTINATION bin)

cmake_minimum_required(VERSION 3.16)
project(fo_omni LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

message(STATUS "fo_omni will use BobUI-provided Qt6 packages when available via CMAKE_PREFIX_PATH / BOBUI_ROOT")

set(OMNI_CORE_DIR ${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/omnicore)
file(GLOB_RECURSE OMNI_CORE_SOURCES CONFIGURE_DEPENDS
    ${OMNI_CORE_DIR}/src/*.cpp
)
list(APPEND OMNI_CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/juce/juce_mock.cpp
)

add_executable(fo_omni
    src/main.cpp
    src/FileModel.cpp
    src/TreemapModel.cpp
    src/NativeMarkdownView.cpp
    ${OMNI_CORE_SOURCES}
    assets/qml.qrc
)

target_include_directories(fo_omni PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OMNI_CORE_DIR}/include
    ${OMNI_CORE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/juce
    ${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/imgui
)

target_link_libraries(fo_omni PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    fo_core
)

# Install rules
install(TARGETS fo_omni DESTINATION bin)

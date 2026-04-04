cmake_minimum_required(VERSION 3.16)
project(fo_omni LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 WebEngineQuick)

add_executable(fo_omni
    src/main.cpp
    src/FileModel.cpp
    src/TreemapModel.cpp
    assets/qml.qrc
)

target_include_directories(fo_omni PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(fo_omni PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::WebEngineQuick
    fo_core
)

# Install rules
install(TARGETS fo_omni DESTINATION bin)

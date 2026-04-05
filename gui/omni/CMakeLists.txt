cmake_minimum_required(VERSION 3.16)
project(fo_omni LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(BTK REQUIRED CONFIG)

fo_resolve_btk_target(Core BTK_CORE_TARGET)
fo_resolve_btk_target(Gui BTK_GUI_TARGET)
fo_resolve_btk_target(Declarative BTK_DECLARATIVE_TARGET)

message(STATUS "fo_omni will use BTK-provided native framework packages when available via CMAKE_PREFIX_PATH / BTK_ROOT")

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
)

target_include_directories(fo_omni PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(fo_omni PRIVATE
    ${BTK_CORE_TARGET}
    ${BTK_GUI_TARGET}
    ${BTK_DECLARATIVE_TARGET}
    fo_core
)

install(TARGETS fo_omni DESTINATION bin)

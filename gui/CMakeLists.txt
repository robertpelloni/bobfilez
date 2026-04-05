if (NOT FO_BUILD_GUI)
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick)

set(BOBUI_OMNICORE_DIR "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/omnicore")
file(GLOB_RECURSE BOBUI_OMNICORE_SOURCES CONFIGURE_DEPENDS
    "${BOBUI_OMNICORE_DIR}/src/*.cpp"
)

message(STATUS "fo_gui will use BobUI Omni wiring and discover Qt6 packages via QT6_ROOT / QT_ROOT / QTDIR / CMAKE_PREFIX_PATH")

set(GUI_SOURCES
    omni/src/main.cpp
    omni/src/NativeUiBootstrap.cpp
    omni/src/NativeUiBootstrap.hpp
    omni/src/NativeUiLaunchProfile.cpp
    omni/src/NativeUiLaunchProfile.hpp
    omni/src/NativeUiProfileRegistry.cpp
    omni/src/NativeUiProfileRegistry.hpp
    omni/src/NativeUiRuntime.cpp
    omni/src/NativeUiRuntime.hpp
    omni/src/NativeUiRuntimeBundle.cpp
    omni/src/NativeUiRuntimeBundle.hpp
    omni/src/OmniQmlRegistration.cpp
    omni/src/OmniQmlRegistration.hpp
    omni/src/FileModel.cpp
    omni/src/FileModel.h
    omni/src/TreemapModel.cpp
    omni/src/TreemapModel.h
    omni/src/NativeMarkdownView.cpp
    omni/src/NativeMarkdownView.h
)

set(QML_ASSETS
    omni/assets/main.qml
    omni/assets/AcrylicBackground.qml
    omni/assets/DesktopIcons.qml
    omni/assets/Dashboard.qml
    omni/assets/StartMenu.qml
)

add_executable(fo_gui 
    ${GUI_SOURCES}
    ${QML_ASSETS}
    ${BOBUI_OMNICORE_SOURCES}
)

target_include_directories(fo_gui PRIVATE 
    omni/src
    "${BOBUI_OMNICORE_DIR}/include"
    "${BOBUI_OMNICORE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/juce"
    "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/imgui"
)

target_link_libraries(fo_gui PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    fo_core
)

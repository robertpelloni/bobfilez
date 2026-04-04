
if (NOT FO_BUILD_GUI)
    return()
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets Qml Quick REQUIRED)

message(STATUS "fo_gui will use BobUI-provided Qt6 packages when available via CMAKE_PREFIX_PATH / BOBUI_ROOT")

# OmniUI Core Sources (Local path relative to gui/)
set(OMNI_CORE_DIR ${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/core)
set(OMNI_CORE_SOURCES
    ${OMNI_CORE_DIR}/src/OmniApp.cpp
    ${OMNI_CORE_DIR}/src/JuceWidget.cpp
    ${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/juce/juce_mock.cpp
)

set(GUI_SOURCES
    omni/src/main.cpp
    omni/src/FileModel.cpp
    omni/src/FileModel.h
    ${OMNI_CORE_SOURCES}
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
)

target_include_directories(fo_gui PRIVATE 
    omni/src 
    ${OMNI_CORE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/juce
)

target_link_libraries(fo_gui PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::Qml 
    Qt6::Quick 
    fo_core
)

# Standard Qt resource handling could go here
# qt_add_resources(fo_gui assets.qrc)

if (NOT FO_BUILD_GUI)
    return()
endif()

find_package(BTK REQUIRED CONFIG)

fo_resolve_btk_target(Core BTK_CORE_TARGET)
fo_resolve_btk_target(Gui BTK_GUI_TARGET)
fo_resolve_btk_target(Declarative BTK_DECLARATIVE_TARGET)

message(STATUS "fo_gui will use BTK-provided native framework packages when available via CMAKE_PREFIX_PATH / BTK_ROOT")

set(GUI_SOURCES
    omni/src/main.cpp
    omni/src/NativeUiBootstrap.cpp
    omni/src/NativeUiBootstrap.hpp
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
)

target_include_directories(fo_gui PRIVATE 
    omni/src
)

target_link_libraries(fo_gui PRIVATE 
    ${BTK_CORE_TARGET}
    ${BTK_GUI_TARGET}
    ${BTK_DECLARATIVE_TARGET}
    fo_core
)

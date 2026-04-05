if (NOT FO_BUILD_BOBUI_DEMO)
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

set(BOBUI_OMNICORE_DIR "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/omnicore")
file(GLOB_RECURSE BOBUI_OMNICORE_SOURCES CONFIGURE_DEPENDS
    "${BOBUI_OMNICORE_DIR}/src/*.cpp"
)

add_executable(fo_bobui_demo
    src/main.cpp
    assets/bobui_demo.qrc
    ${BOBUI_OMNICORE_SOURCES}
)

target_include_directories(fo_bobui_demo PRIVATE
    src
    "${BOBUI_OMNICORE_DIR}/include"
    "${BOBUI_OMNICORE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/juce"
    "${CMAKE_SOURCE_DIR}/libs/bobui/OmniUI/deps/imgui"
)

target_link_libraries(fo_bobui_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    fo_core
)

install(TARGETS fo_bobui_demo DESTINATION bin)

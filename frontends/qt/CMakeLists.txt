if (NOT FO_BUILD_QT_DEMO)
    return()
endif()

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(fo_qt_demo
    src/main.cpp
)

target_link_libraries(fo_qt_demo PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    fo_core
)

install(TARGETS fo_qt_demo DESTINATION bin)

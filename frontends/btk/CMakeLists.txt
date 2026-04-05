if (NOT FO_BUILD_BTK_DEMO)
    return()
endif()

find_package(BTK CONFIG REQUIRED COMPONENTS Core Gui)

add_executable(fo_btk_demo
    src/main.cpp
)

target_link_libraries(fo_btk_demo PRIVATE
    BTK::Core
    BTK::Gui
)

install(TARGETS fo_btk_demo DESTINATION bin)

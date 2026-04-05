if (NOT FO_BUILD_JUCE_DEMO)
    return()
endif()

set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_MODULES_ONLY OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/JUCE ${CMAKE_BINARY_DIR}/libs/JUCE EXCLUDE_FROM_ALL)

juce_add_gui_app(fo_juce_demo
    PRODUCT_NAME "bobfilez JUCE Demo"
    COMPANY_NAME "bobfilez"
    VERSION "${FO_VERSION_STRING}"
)

juce_generate_juce_header(fo_juce_demo)

target_sources(fo_juce_demo PRIVATE
    src/main.cpp
)

target_link_libraries(fo_juce_demo PRIVATE
    fo_core
    juce::juce_gui_extra
)

target_compile_definitions(fo_juce_demo PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

install(TARGETS fo_juce_demo DESTINATION bin)

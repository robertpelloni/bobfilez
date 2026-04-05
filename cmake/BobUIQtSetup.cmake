# Prefer BobUI when GUI / Omni targets are enabled.
#
# BobUI is a Qt fork and should be treated as a Qt6 package provider.
# bobfilez's native GUI targets therefore still consume normal Qt6::*
# targets; BobUI changes where those packages come from.

set(FO_BOBUI_ROOT "${CMAKE_SOURCE_DIR}/libs/bobui" CACHE PATH
    "Path to the BobUI source/build/install root")

function(fo_configure_bobui_qt)
    set(_bobui_roots)

    if(DEFINED ENV{BOBUI_ROOT} AND NOT "$ENV{BOBUI_ROOT}" STREQUAL "")
        list(APPEND _bobui_roots "$ENV{BOBUI_ROOT}")
    endif()

    if(FO_BOBUI_ROOT)
        list(APPEND _bobui_roots "${FO_BOBUI_ROOT}")
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/libs/bobui")
        list(APPEND _bobui_roots "${CMAKE_SOURCE_DIR}/libs/bobui")
    endif()

    list(REMOVE_DUPLICATES _bobui_roots)

    set(_bobui_prefix_hints)
    foreach(_bobui_root IN LISTS _bobui_roots)
        list(APPEND _bobui_prefix_hints
            "${_bobui_root}/build-bobui"
            "${_bobui_root}/build-bobui/install"
            "${_bobui_root}/build"
            "${_bobui_root}/build/install"
            "${_bobui_root}/out"
            "${_bobui_root}/out/install"
            "${_bobui_root}/install"
            "${_bobui_root}/lib/cmake"
            "${_bobui_root}/lib/cmake/Qt6"
        )
    endforeach()

    foreach(_bobui_hint IN LISTS _bobui_prefix_hints)
        if(EXISTS "${_bobui_hint}")
            list(APPEND CMAKE_PREFIX_PATH "${_bobui_hint}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)

    if(EXISTS "${CMAKE_SOURCE_DIR}/libs/bobui")
        message(STATUS "BobUI detected at ${CMAKE_SOURCE_DIR}/libs/bobui; preferring it as the Qt6 provider for GUI/Omni targets")
    else()
        message(STATUS "BobUI source tree not found under libs/bobui; falling back to standard Qt6 package discovery")
    endif()
endfunction()

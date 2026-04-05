# Prefer BTK when GUI / Omni targets are enabled.
#
# BTK is maintained at:
#   https://github.com/robertpelloni/btk
#
# bobfilez's native GUI path is being retargeted from the older BobUI/Qt-fork
# assumptions toward BTK's package model. BTK is not a Qt6 drop-in provider;
# it exposes BTK / CopperSpice CMake packages instead.

set(FO_BTK_ROOT "${CMAKE_SOURCE_DIR}/libs/btk" CACHE PATH
    "Path to the BTK source/build/install root")

function(fo_configure_btk_framework)
    set(_btk_roots)

    if(DEFINED ENV{BTK_ROOT} AND NOT "$ENV{BTK_ROOT}" STREQUAL "")
        list(APPEND _btk_roots "$ENV{BTK_ROOT}")
    endif()

    if(FO_BTK_ROOT)
        list(APPEND _btk_roots "${FO_BTK_ROOT}")
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/libs/btk")
        list(APPEND _btk_roots "${CMAKE_SOURCE_DIR}/libs/btk")
    endif()

    list(REMOVE_DUPLICATES _btk_roots)

    set(_btk_prefix_hints)
    foreach(_btk_root IN LISTS _btk_roots)
        list(APPEND _btk_prefix_hints
            "${_btk_root}/build-btk"
            "${_btk_root}/build-btk/install"
            "${_btk_root}/build"
            "${_btk_root}/build/install"
            "${_btk_root}/out"
            "${_btk_root}/out/install"
            "${_btk_root}/install"
            "${_btk_root}/lib/cmake"
            "${_btk_root}/lib/cmake/BTK"
            "${_btk_root}/lib/cmake/CopperSpice"
        )
    endforeach()

    foreach(_btk_hint IN LISTS _btk_prefix_hints)
        if(EXISTS "${_btk_hint}")
            list(APPEND CMAKE_PREFIX_PATH "${_btk_hint}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)

    if(EXISTS "${CMAKE_SOURCE_DIR}/libs/btk")
        message(STATUS "BTK detected at ${CMAKE_SOURCE_DIR}/libs/btk; preferring it as the native GUI framework provider for GUI/Omni targets")
    else()
        message(STATUS "BTK source tree not found under libs/btk; falling back to standard BTK/CopperSpice package discovery")
    endif()
endfunction()

function(fo_resolve_btk_target component out_var)
    set(_candidate_targets
        "BTK::${component}"
        "BTK::Btk${component}"
        "CopperSpice::Cs${component}"
    )

    foreach(_candidate IN LISTS _candidate_targets)
        if(TARGET ${_candidate})
            set(${out_var} ${_candidate} PARENT_SCOPE)
            return()
        endif()
    endforeach()

    message(FATAL_ERROR "Could not resolve a BTK/CopperSpice target for component '${component}'")
endfunction()

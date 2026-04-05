# Configure the active BobUI / Omni integration path when GUI / Omni targets are enabled.
#
# bobfilez still consumes normal Qt6::* targets, but BobUI contributes the OmniUI
# layer and may optionally contribute some Qt package roots when a compatible build
# tree exists. In practice, Qt6 Qml / Quick may still need to come from an external
# Qt installation because the local BobUI tree here does not include qtdeclarative
# sources in-tree.

set(FO_BOBUI_ROOT "${CMAKE_SOURCE_DIR}/libs/bobui" CACHE PATH
    "Path to the BobUI source/build/install root")

function(fo_configure_bobui_qt)
    set(_qt_roots)
    set(_bobui_roots)

    foreach(_qt_root_var IN ITEMS QT6_ROOT QT_ROOT QTDIR)
        if(DEFINED ENV{${_qt_root_var}} AND NOT "$ENV{${_qt_root_var}}" STREQUAL "")
            list(APPEND _qt_roots "$ENV{${_qt_root_var}}")
        endif()
    endforeach()

    if(DEFINED ENV{BOBUI_ROOT} AND NOT "$ENV{BOBUI_ROOT}" STREQUAL "")
        list(APPEND _bobui_roots "$ENV{BOBUI_ROOT}")
    endif()

    if(FO_BOBUI_ROOT)
        list(APPEND _bobui_roots "${FO_BOBUI_ROOT}")
    endif()

    if(EXISTS "${CMAKE_SOURCE_DIR}/libs/bobui")
        list(APPEND _bobui_roots "${CMAKE_SOURCE_DIR}/libs/bobui")
    endif()

    list(REMOVE_DUPLICATES _qt_roots)
    list(REMOVE_DUPLICATES _bobui_roots)

    set(_qt_prefix_hints)
    foreach(_qt_root IN LISTS _qt_roots)
        list(APPEND _qt_prefix_hints
            "${_qt_root}"
            "${_qt_root}/lib/cmake"
            "${_qt_root}/lib/cmake/Qt6"
        )
    endforeach()

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

    foreach(_qt_hint IN LISTS _qt_prefix_hints)
        if(EXISTS "${_qt_hint}")
            list(APPEND CMAKE_PREFIX_PATH "${_qt_hint}")
        endif()
    endforeach()

    foreach(_bobui_hint IN LISTS _bobui_prefix_hints)
        if(EXISTS "${_bobui_hint}")
            list(APPEND CMAKE_PREFIX_PATH "${_bobui_hint}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)

    if(EXISTS "${CMAKE_SOURCE_DIR}/libs/bobui")
        message(STATUS "BobUI detected at ${CMAKE_SOURCE_DIR}/libs/bobui; using BobUI Omni wiring while searching for Qt6 package roots via QT6_ROOT / QT_ROOT / QTDIR / CMAKE_PREFIX_PATH")
    else()
        message(STATUS "BobUI source tree not found under libs/bobui; falling back to standard Qt6 package discovery")
    endif()
endfunction()

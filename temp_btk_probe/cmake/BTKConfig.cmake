# *****************************************************************************
# BTK CMake package configuration
#
# This wrapper preserves the existing CopperSpice target exports while exposing
# BTK-branded imported target aliases for downstream consumers.
# *****************************************************************************

if (BTK_FOUND)
   return()
endif()

set(BTK_FOUND TRUE)

get_filename_component(BTK_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(BTK_PREFIX ${BTK_CMAKE_DIR}/ ABSOLUTE)

include("${BTK_CMAKE_DIR}/CopperSpiceLibraryTargets.cmake")
include("${BTK_CMAKE_DIR}/CopperSpiceBinaryTargets.cmake")
include("${BTK_CMAKE_DIR}/BTKMacros.cmake")
include("${BTK_CMAKE_DIR}/BTKDeploy.cmake")

set(BTK_INCLUDES @CMAKE_INSTALL_FULL_INCLUDEDIR@)
set(BTK_LIBRARIES)
set(BTK_COMPONENTS @BUILD_COMPONENTS@)

foreach(component ${BTK_COMPONENTS})
   string(TOUPPER ${component} uppercomp)

   if (TARGET CopperSpice::Cs${component})
      if (NOT TARGET BTK::Btk${component})
         add_library(BTK::Btk${component} ALIAS CopperSpice::Cs${component})
      endif()

      if (NOT TARGET BTK::${component})
         add_library(BTK::${component} ALIAS CopperSpice::Cs${component})
      endif()
   endif()

   set(BTK_INCLUDES
      ${BTK_INCLUDES}
      @CMAKE_INSTALL_FULL_INCLUDEDIR@/Qt${component}
   )

   set(BTK_${uppercomp}_INCLUDES
      @CMAKE_INSTALL_FULL_INCLUDEDIR@/Qt${component}
   )

   set(BTK_LIBRARIES
      ${BTK_LIBRARIES}
      BTK::Btk${component}
   )

   set(BTK_${uppercomp}_LIBRARIES
      BTK::Btk${component}
   )

   set(BTK_${uppercomp}_TARGET BTK::${component})
endforeach()

set(BTK_INSTALL_MODE     "@CS_INSTALL_MODE@")
set(BtkLibGuarded_Deploy "@CsLibGuarded_FOUND@")
set(BtkPointer_Deploy    "@CsPointer_FOUND@")
set(BtkSignal_Deploy     "@CsSignal_FOUND@")
set(BtkString_Deploy     "@CsString_FOUND@")

set(BTK_VERSION_MAJOR "@BUILD_MAJOR@")
set(BTK_VERSION_MINOR "@BUILD_MINOR@")
set(BTK_VERSION_PATCH "@BUILD_MICRO@")
set(BTK_VERSION       "@BUILD_MAJOR@.@BUILD_MINOR@.@BUILD_MICRO@")
set(BTK_VERSION_API   "@BUILD_MAJOR@.@BUILD_MINOR@")

set(BTK_CMAKE_MACROS_FILE "${BTK_CMAKE_DIR}/BTKMacros.cmake")
set(BTK_CMAKE_DEPLOY_FILE "${BTK_CMAKE_DIR}/BTKDeploy.cmake")

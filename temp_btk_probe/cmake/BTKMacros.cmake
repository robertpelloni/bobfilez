# *****************************************************************************
# BTK macro compatibility wrappers
# *****************************************************************************

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/CopperSpiceMacros.cmake")

macro(BTK_RESOURCES RESOURCES)
   COPPERSPICE_RESOURCES(${RESOURCES} ${ARGN})
endmacro()

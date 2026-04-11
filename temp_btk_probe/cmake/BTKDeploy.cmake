# *****************************************************************************
# BTK deploy compatibility wrappers
# *****************************************************************************

include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/CopperSpiceDeploy.cmake")

function(btk_copy_library LIB_NAME)
   cs_copy_library(${LIB_NAME})
endfunction()

function(btk_copy_plugins LIB_NAME)
   cs_copy_plugins(${LIB_NAME})
endfunction()

# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "frontends\\qt\\CMakeFiles\\fo_qt_demo_autogen.dir\\AutogenUsed.txt"
  "frontends\\qt\\CMakeFiles\\fo_qt_demo_autogen.dir\\ParseCache.txt"
  "frontends\\qt\\fo_qt_demo_autogen"
  )
endif()

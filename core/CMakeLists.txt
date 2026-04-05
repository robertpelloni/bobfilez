set(FO_CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libs)

# Read version from VERSION.md
file(READ "${CMAKE_SOURCE_DIR}/VERSION.md" FO_VERSION_STRING)
string(STRIP "${FO_VERSION_STRING}" FO_VERSION_STRING)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fo/core/version.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fo/core/version.hpp
    @ONLY
)

# Vendored libraries: TinyEXIF, hash-library (SHA256/etc), xxHash
set(TINYEXIF_SOURCES ${LIBS_DIR}/TinyEXIF/TinyEXIF.cpp)
set(TINYXML2_SOURCES ${LIBS_DIR}/tinyxml2/tinyxml2.cpp)
set(HASHLIBRARY_SOURCES
    ${LIBS_DIR}/hash-library/sha256.cpp
    ${LIBS_DIR}/hash-library/sha1.cpp
    ${LIBS_DIR}/hash-library/md5.cpp
    ${LIBS_DIR}/hash-library/crc32.cpp
    ${LIBS_DIR}/hash-library/keccak.cpp
    ${LIBS_DIR}/hash-library/sha3.cpp
)
# xxHash is header-only if XXH_INLINE_ALL is defined; no separate .c needed
set(XXHASH_INCLUDE ${LIBS_DIR}/xxHash)

# md4c (vendored markdown parser / HTML renderer)
set(MD4C_SOURCES
    ${LIBS_DIR}/md4c/src/md4c.c
    ${LIBS_DIR}/md4c/src/md4c-html.c
    ${LIBS_DIR}/md4c/src/entity.c
)

# SQLite3
set(SQLITE3_SOURCES ${LIBS_DIR}/sqlite3/sqlite3.c)

# Dirent (Windows shim)
if(WIN32)
    set(DIRENT_SOURCES ${LIBS_DIR}/dirent/src/dirent.c)
endif()

file(GLOB_RECURSE FO_CORE_HEADERS ${FO_CORE_INCLUDE_DIR}/*.hpp)
file(GLOB_RECURSE FO_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(APPEND FO_CORE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/duplicate_finders.cpp)

# Find optional provider dependencies
find_package(exiv2 CONFIG QUIET)
find_package(blake3 CONFIG)
find_package(Tesseract CONFIG QUIET)
find_package(OpenCV CONFIG QUIET)
find_package(onnxruntime CONFIG QUIET)
find_package(yaml-cpp CONFIG QUIET)
find_package(AWSSDK QUIET COMPONENTS s3 core)
find_package(cpr CONFIG QUIET)
find_package(azure-storage-blobs-cpp CONFIG QUIET)
find_package(FFmpeg QUIET COMPONENTS avformat avcodec avutil swscale)
find_package(chromaprint CONFIG QUIET)

if(exiv2_FOUND)
    message(STATUS "Found exiv2, enabling exiv2 metadata provider")
    # Source file is already in FO_CORE_SOURCES via GLOB
endif()

if(blake3_FOUND)
    message(STATUS "Found BLAKE3, enabling blake3 hasher")
    # Source file is already in FO_CORE_SOURCES via GLOB
endif()

if(Tesseract_FOUND)
    message(STATUS "Found Tesseract, enabling tesseract OCR provider")
    # Source file is already in FO_CORE_SOURCES via GLOB
endif()

if(OpenCV_FOUND)
    message(STATUS "Found OpenCV, enabling perceptual hash provider")
endif()

if(onnxruntime_FOUND)
    message(STATUS "Found onnxruntime, enabling AI classification provider")
endif()

if(AWSSDK_FOUND)
    message(STATUS "Found AWSSDK, enabling AWS S3 provider")
endif()

if(cpr_FOUND)
    message(STATUS "Found cpr, enabling Google Drive HTTP provider")
endif()

if(azure-storage-blobs-cpp_FOUND)
    message(STATUS "Found Azure Storage Blobs SDK, enabling Azure Blob provider")
endif()

add_library(fo_core STATIC
    ${FO_CORE_HEADERS}
    ${FO_CORE_SOURCES}
    ${TINYEXIF_SOURCES}
    ${TINYXML2_SOURCES}
    ${HASHLIBRARY_SOURCES}
    ${MD4C_SOURCES}
    ${SQLITE3_SOURCES}
    ${DIRENT_SOURCES}
)

add_library(fo_c_api STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include/fo/c_api/bobfilez_c_api.h
    ${CMAKE_CURRENT_SOURCE_DIR}/c_api/bobfilez_c_api.cpp
)

target_include_directories(fo_core
    PUBLIC ${FO_CORE_INCLUDE_DIR}
    PRIVATE ${LIBS_DIR}/TinyEXIF
    PRIVATE ${LIBS_DIR}/tinyxml2
    PRIVATE ${LIBS_DIR}/hash-library
    PRIVATE ${XXHASH_INCLUDE}
    PRIVATE ${LIBS_DIR}/md4c/src
    PRIVATE ${LIBS_DIR}/dirent/src
    PRIVATE ${LIBS_DIR}/sqlite3
    PRIVATE ${LIBS_DIR}
)

if(exiv2_FOUND)
    target_compile_definitions(fo_core PRIVATE FO_HAVE_EXIV2)
    target_link_libraries(fo_core PUBLIC exiv2::exiv2lib)
endif()

if(blake3_FOUND)
    target_compile_definitions(fo_core PRIVATE FO_HAVE_BLAKE3)
    target_link_libraries(fo_core PUBLIC BLAKE3::blake3)
endif()

if(Tesseract_FOUND)
    target_compile_definitions(fo_core PRIVATE FO_HAVE_TESSERACT)
    target_link_libraries(fo_core PUBLIC Tesseract::libtesseract)
endif()

if(OpenCV_FOUND)
    target_compile_definitions(fo_core PRIVATE FO_HAVE_OPENCV)
    target_link_libraries(fo_core PUBLIC ${OpenCV_LIBS})
    target_include_directories(fo_core PRIVATE ${OpenCV_INCLUDE_DIRS})
endif()

if(onnxruntime_FOUND)
    target_compile_definitions(fo_core PRIVATE FO_HAVE_ONNXRUNTIME)
    target_link_libraries(fo_core PUBLIC onnxruntime::onnxruntime)
endif()

if(yaml-cpp_FOUND)
    target_compile_definitions(fo_core PRIVATE FO_HAVE_YAMLCPP)
    target_link_libraries(fo_core PUBLIC yaml-cpp::yaml-cpp)
endif()

if(AWSSDK_FOUND)
    target_compile_definitions(fo_core PUBLIC FO_HAVE_S3)
    target_link_libraries(fo_core PUBLIC ${AWSSDK_LINK_LIBRARIES})
endif()

if(cpr_FOUND)
    target_compile_definitions(fo_core PRIVATE FO_HAVE_CPR)
    target_link_libraries(fo_core PUBLIC cpr::cpr)
endif()

if(azure-storage-blobs-cpp_FOUND)
    target_compile_definitions(fo_core PRIVATE FO_HAVE_AZURE_STORAGE)
    target_link_libraries(fo_core PUBLIC Azure::azure-storage-blobs)
endif()

if(FFmpeg_FOUND)
    message(STATUS "Found FFmpeg, enabling video hashing provider")
    target_compile_definitions(fo_core PUBLIC FO_HAVE_FFMPEG)
    target_link_libraries(fo_core PUBLIC FFmpeg::avformat FFmpeg::avcodec FFmpeg::avutil FFmpeg::swscale)
endif()

if(chromaprint_FOUND)
    message(STATUS "Found Chromaprint, enabling audio fingerprint provider")
    target_compile_definitions(fo_core PUBLIC FO_HAVE_CHROMAPRINT)
    target_link_libraries(fo_core PUBLIC chromaprint::chromaprint)
endif()

target_include_directories(fo_c_api
    PUBLIC ${FO_CORE_INCLUDE_DIR}
)

target_link_libraries(fo_c_api PUBLIC fo_core)

target_compile_features(fo_core PUBLIC cxx_std_20)
target_compile_features(fo_c_api PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(fo_core PRIVATE /W4 /permissive-)
  target_compile_options(fo_c_api PRIVATE /W4 /permissive-)
else()
  target_compile_options(fo_core PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(fo_c_api PRIVATE -Wall -Wextra -Wpedantic)
endif()

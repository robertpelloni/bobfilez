cmake_minimum_required(VERSION 3.18)
project(btk_package_smoke LANGUAGES CXX)

find_package(BTK REQUIRED COMPONENTS Core)

add_executable(btk_package_smoke main.cpp)
target_link_libraries(btk_package_smoke PRIVATE BTK::Core)

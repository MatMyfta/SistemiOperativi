cmake_minimum_required(VERSION 3.0)
# Export the compile_commands.json file, which is used by language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Default build type: Debug
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Default to debug mode" FORCE)
endif()

project(sistemi-operativi)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_compile_options(-std=gnu90 -Wall -Wextra -Wno-unused)

add_subdirectory(src)

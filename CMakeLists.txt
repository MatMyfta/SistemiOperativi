cmake_minimum_required(VERSION 3.0)
# Export the compile_commands.json file, which is used by language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Default build type: Debug
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Default to debug mode" FORCE)
endif()

project(sistemi-operativi)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90 -Wall -Wextra -Wno-unused")

# include Catch2
add_subdirectory(third-party/Catch2)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/third-party/Catch2/contrib")
include(Catch)

# Enable CMake testing features
enable_testing()

add_subdirectory(src)

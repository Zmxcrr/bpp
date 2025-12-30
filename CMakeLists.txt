cmake_minimum_required(VERSION 3.15)
project(BaldPlusPlus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Feature options (можно переопределить при вызове cmake)
option(BALD_ENABLE_VM "Enable bytecode VM" ON)
option(BALD_ENABLE_GC "Enable garbage collector" ON)
option(BALD_ENABLE_OPTIMIZATIONS "Enable bytecode optimizations" ON)

# Library
add_subdirectory(lib)

# Executable
add_executable(bald_interpreter main.cpp)
target_link_libraries(bald_interpreter PRIVATE bald_lib)

# Examples
add_subdirectory(examples EXCLUDE_FROM_ALL)

# Print configuration summary
message(STATUS "")
message(STATUS "=== Bald++ Configuration ===")
message(STATUS "  VM:            ${BALD_ENABLE_VM}")
message(STATUS "  GC:            ${BALD_ENABLE_GC}")
message(STATUS "  Optimizations: ${BALD_ENABLE_OPTIMIZATIONS}")
message(STATUS "  Build type:    ${CMAKE_BUILD_TYPE}")
message(STATUS "===========================")
message(STATUS "")


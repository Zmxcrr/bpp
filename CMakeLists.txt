cmake_minimum_required(VERSION 3.20)
project(BaldPlusPlus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -O3 -march=native)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")  
    add_compile_options(-Wall -Wextra -O3 -march=native)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4 /O2 /std:c++latest)
endif()

option(ENABLE_GC "Enable garbage collection" ON)
option(ENABLE_SIMPLE_JIT "Enable Simple JIT compiler" ON)
option(BUILD_EXAMPLES "Build examples" ON)

add_subdirectory(lib)

add_executable(interpreter main.cpp)
target_link_libraries(interpreter PRIVATE bald_interpreter)

if(BUILD_EXAMPLES AND EXISTS "${CMAKE_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/programs")

if(EXISTS "${CMAKE_SOURCE_DIR}/examples")
    file(GLOB BALD_EXAMPLES "${CMAKE_SOURCE_DIR}/examples/*.bald")
    foreach(BALD_FILE ${BALD_EXAMPLES})
        file(COPY ${BALD_FILE} DESTINATION "${CMAKE_BINARY_DIR}/programs")
    endforeach()
endif()

install(TARGETS interpreter DESTINATION bin)

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Bald++ Interpreter Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "GC: ${ENABLE_GC}")
message(STATUS "JIT: ${ENABLE_SIMPLE_JIT}")
message(STATUS "Examples: ${BUILD_EXAMPLES}")
message(STATUS "========================================")
message(STATUS "")


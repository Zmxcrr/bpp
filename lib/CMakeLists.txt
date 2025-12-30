option(BALD_ENABLE_VM "Enable bytecode VM" ON)
option(BALD_ENABLE_GC "Enable garbage collector" ON)
option(BALD_ENABLE_OPTIMIZATIONS "Enable bytecode optimizations" ON)

set(INTERPRETER_SOURCES
    interpreter/lexer/lexer.cpp
    interpreter/parser/parser.cpp
    interpreter/expression/expression.cpp
    interpreter/statement/statement.cpp
    interpreter/interpreter/interpreter.cpp
)

add_library(bald_lib ${INTERPRETER_SOURCES})

target_include_directories(bald_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter
)

target_compile_features(bald_lib PUBLIC cxx_std_23)

# VM (опционально)
if(BALD_ENABLE_VM)
    target_sources(bald_lib PRIVATE
        vm/vm.cpp
        compiler/compiler.cpp
    )
    target_compile_definitions(bald_lib PUBLIC BALD_HAS_VM)
    message(STATUS "Bald++: VM enabled")
endif()

# GC (опционально)
if(BALD_ENABLE_GC)
    target_sources(bald_lib PRIVATE
        gc/gc.cpp
    )
    target_compile_definitions(bald_lib PUBLIC BALD_HAS_GC)
    message(STATUS "Bald++: GC enabled")
endif()

# Optimizers (опционально, требует VM)
if(BALD_ENABLE_OPTIMIZATIONS AND BALD_ENABLE_VM)
    target_sources(bald_lib PRIVATE
        optimizer/const_folding.cpp
        optimizer/dead_code.cpp
    )
    target_compile_definitions(bald_lib PUBLIC BALD_HAS_OPTIMIZATIONS)
    message(STATUS "Bald++: Bytecode optimizations enabled")
elseif(BALD_ENABLE_OPTIMIZATIONS AND NOT BALD_ENABLE_VM)
    message(WARNING "Bald++: Optimizations require VM, disabling optimizations")
endif()

# Compiler options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bald_lib PRIVATE
        -Wall -Wextra -Wpedantic
    )
    if(CMAKE_BUILD_TYPE MATCHES Release)
        target_compile_options(bald_lib PRIVATE
            -O3 -march=native
        )
    endif()
endif()

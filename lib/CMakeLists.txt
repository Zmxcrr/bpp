# lib/CMakeLists.txt

add_library(bald_interpreter STATIC
    interpreter/expression/expression.cpp
    interpreter/lexer/lexer.cpp
    interpreter/parser/parser.cpp
    interpreter/statement/statement.cpp
    interpreter/interpreter/interpreter.cpp
)

if(ENABLE_SIMPLE_JIT)
    target_sources(bald_interpreter PRIVATE simple_jit.cpp)
    target_compile_definitions(bald_interpreter PUBLIC HAS_SIMPLE_JIT=1)
endif()

if(ENABLE_GC)
    target_compile_definitions(bald_interpreter PUBLIC HAS_GC=1)
endif()

target_include_directories(bald_interpreter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/value
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/environment
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/functiondef
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/token
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/exception
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/expression
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/statement
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter/interpreter
)

target_compile_features(bald_interpreter PUBLIC cxx_std_20)


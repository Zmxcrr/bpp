message(STATUS "Processing Bald++ examples...")

file(GLOB CPP_FILES "*.cpp")
if(CPP_FILES)
    foreach(CPP_FILE ${CPP_FILES})
        get_filename_component(EXEC_NAME ${CPP_FILE} NAME_WE)
        add_executable(${EXEC_NAME} ${CPP_FILE})
        target_link_libraries(${EXEC_NAME} interpreter_lib)

        if(HAS_SIMPLE_JIT)
            target_compile_definitions(${EXEC_NAME} PRIVATE HAS_SIMPLE_JIT=1)
        endif()

        message(STATUS "Added C++ example: ${EXEC_NAME}")
    endforeach()
endif()

file(GLOB BALD_FILES "*.bald")

if(BALD_FILES)
    message(STATUS "Found Bald++ programs (.bald files):")
    foreach(BALD_FILE ${BALD_FILES})
        get_filename_component(BALD_NAME ${BALD_FILE} NAME)
        message(STATUS "  - ${BALD_NAME}")
    endforeach()

    foreach(BALD_FILE ${BALD_FILES})
        get_filename_component(BALD_NAME ${BALD_FILE} NAME)
        configure_file(${BALD_FILE} "${CMAKE_BINARY_DIR}/programs/${BALD_NAME}" COPYONLY)
    endforeach()

    foreach(BALD_FILE ${BALD_FILES})
        get_filename_component(BALD_NAME_WE ${BALD_FILE} NAME_WE)

        add_custom_target(run_${BALD_NAME_WE}
            COMMAND ${CMAKE_BINARY_DIR}/interpreter ${CMAKE_BINARY_DIR}/programs/${BALD_NAME_WE}.bald
            DEPENDS interpreter
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running Bald++ program: ${BALD_NAME_WE}.bald"
        )
    endforeach()

    set(ALL_BALD_TARGETS "")
    foreach(BALD_FILE ${BALD_FILES})
        get_filename_component(BALD_NAME_WE ${BALD_FILE} NAME_WE)
        list(APPEND ALL_BALD_TARGETS run_${BALD_NAME_WE})
    endforeach()

    add_custom_target(run_all_programs
        DEPENDS ${ALL_BALD_TARGETS}
        COMMENT "Running all Bald++ programs"
    )

    add_custom_target(list_programs
        COMMAND ${CMAKE_COMMAND} -E echo "Available Bald++ programs:"
        COMMAND ${CMAKE_COMMAND} -E echo "=========================="
    )

    foreach(BALD_FILE ${BALD_FILES})
        get_filename_component(BALD_NAME ${BALD_FILE} NAME)
        get_filename_component(BALD_NAME_WE ${BALD_FILE} NAME_WE)
        add_custom_command(
            TARGET list_programs POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "  make run_${BALD_NAME_WE}    # Run ${BALD_NAME}"
        )
    endforeach()

    add_custom_command(
        TARGET list_programs POST_BUILD  
        COMMAND ${CMAKE_COMMAND} -E echo ""
        COMMAND ${CMAKE_COMMAND} -E echo "Or run directly:"
        COMMAND ${CMAKE_COMMAND} -E echo "  ./interpreter programs/factorial_test.bald"
        COMMAND ${CMAKE_COMMAND} -E echo "  ./interpreter programs/comprehensive_benchmark.bald"
    )

else()
    message(STATUS "No .bald files found in examples/")
endif()


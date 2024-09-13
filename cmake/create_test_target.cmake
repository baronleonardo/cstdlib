function(create_test_target target)
    string(TOUPPER ${target} target_upper)
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/${target}.c")
        file(WRITE "${CMAKE_BINARY_DIR}/${target}.c"
            "#define CSTDLIB_${target_upper}_IMPLEMENTATION\n"
            "#define CSTDLIB_${target_upper}_UNIT_TESTS\n"
            "#include \"${target}.h\"\n")
    endif()

    add_executable(test_${target} ${target}.c ${target}.h)
    target_include_directories(test_${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME test_${target}
        COMMAND test_${target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()
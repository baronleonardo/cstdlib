include("cmake/create_test_target.cmake")

cmake_minimum_required(VERSION 3.15)
project(cstdlib C)

# compiler options
set(CMAKE_C_STANDARD 99)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (MSVC)
    # warning level 4
    add_compile_options(
        /W3
        /WX
        /utf-8
        /Zi
        # $<$<CONFIG:DEBUG>:/fsanitize=address>
        # $<$<CONFIG:DEBUG>:/RTCcsu>
    )
    add_link_options(
        $<$<CONFIG:DEBUG>:/INCREMENTAL:NO>
    )
else()
    # additional warnings
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        $<$<CONFIG:RELEASE>:-D_FORTIFY_SOURCE=2>
        $<$<CONFIG:DEBUG>:-fsanitize=address,undefined>
        $<$<CONFIG:DEBUG>:-fstack-protector-strong>
        $<$<CONFIG:DEBUG>:--param=ssp-buffer-size=4>
        $<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>
        $<$<AND:$<CONFIG:DEBUG>,$<PLATFORM_ID:Linux>>:-fstack-clash-protection>
    )
    add_link_options($<$<CONFIG:DEBUG>:-fsanitize=address,undefined>)
endif()

option(ENABLE_TESTS "Enable tests" OFF)
if (${ENABLE_TESTS})
    enable_testing()
    create_test_target(fs)
    create_test_target(dl_loader)
endif()


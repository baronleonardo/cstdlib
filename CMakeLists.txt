cmake_minimum_required(VERSION 3.15)
project(cstdlib)

# compiler options
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (MSVC)
    # warning level 4
    add_compile_options(
        /W4
        /WX
        /utf-8
        /Zi
        $<$<CONFIG:DEBUG>:/fsanitize=address>
        # $<$<CONFIG:DEBUG>:/RTCcsu>
    )
    add_link_options(
        $<$<CONFIG:DEBUG>:/INCREMENTAL:NO>
    )
else()
    # additional warnings
    add_compile_options(
        -Wall
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

FILE(GLOB_RECURSE modules *.h)
add_executable(${PROJECT_NAME} main.c ${modules})
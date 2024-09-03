cmake_minimum_required(VERSION 3.15)
project(cstdlib)

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

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2 SDL2_ttf SDL2_image)
# pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3 vulkan)

FILE(GLOB_RECURSE modules *.h)
add_executable(${PROJECT_NAME} main.c ${modules})
# target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::SDL2 libglfw.so.3)
target_compile_definitions(${PROJECT_NAME} PRIVATE CURRENT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

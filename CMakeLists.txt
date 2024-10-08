cmake_minimum_required(VERSION 3.28)
project(deadlock C)

set(CMAKE_C_STANDARD 17)

add_executable(deadlock src/main.c
        src/bones.h
        src/buttons.h
        src/config.h
        src/constants.h
        src/features.c
        src/features.h
        src/game.c
        src/game.h
        src/memory.c
        src/memory.h
        src/mouse.c
        src/mouse.h
        src/offsets.c
        src/offsets.h
        src/types.h
        src/vecmath.h
        src/vecmath.c
        src/weapons.h)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(deadlock PRIVATE "-g3" "-Wall" "-Wextra" "-Wconversion" "-Wdouble-promotion" "-Wno-unused-parameter" "-Wno-unused-function"
    "-Wno-sign-conversion" "-fsanitize=undefined,address")
    target_link_options(deadlock PRIVATE "-fsanitize=undefined,address")
else()
    target_compile_options(deadlock PRIVATE "-O3")
endif()

find_library(LIBUDEV udev)
target_link_libraries(deadlock PRIVATE ${LIBUDEV})
target_link_libraries(deadlock PRIVATE m)

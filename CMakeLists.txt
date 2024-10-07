cmake_minimum_required(VERSION 3.28)
project(deadlock C)

set(CMAKE_C_STANDARD 17)

add_executable(deadlock src/main.c
        src/types.h
        src/constants.h
        src/config.h
        src/memory.h
        src/memory.c
        src/offsets.h
        src/offsets.c
        src/features.h
        src/features.c
        src/mouse.h
        src/mouse.c
        src/game.h
        src/game.c
        src/buttons.h
        src/bones.h
        src/vecmath.h
        src/vecmath.c)

find_library(LIBUDEV udev)
target_link_libraries(deadlock PRIVATE ${LIBUDEV})
target_link_libraries(deadlock PRIVATE m)

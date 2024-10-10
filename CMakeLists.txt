cmake_minimum_required(VERSION 3.28)
project(deadlocked C)

set(CMAKE_C_STANDARD 17)

add_executable(deadlocked src/main.c
        src/bones.h
        src/buttons.h
        src/config.c
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

if (CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(deadlocked PRIVATE "-g3" "-Wall" "-Wextra" "-Wconversion" "-Wdouble-promotion" "-Wno-unused-parameter" "-Wno-unused-function"
    "-Wno-sign-conversion" "-fsanitize=undefined,address")
    target_link_options(deadlocked PRIVATE "-fsanitize=undefined,address")
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(deadlocked PRIVATE "-O3" "-flto")
endif()

find_library(LIBUDEV udev)
target_link_libraries(deadlocked PRIVATE ${LIBUDEV})
target_link_libraries(deadlocked PRIVATE m)

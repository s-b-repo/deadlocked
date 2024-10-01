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
        src/features.h
        src/features.c
        src/serial.h
        src/serial.c)

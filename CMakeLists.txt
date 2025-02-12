cmake_minimum_required(VERSION 3.28)
project(deadlocked CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/JetBrainsMono.ttf DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)

add_executable(deadlocked ${SOURCE_FILES})
target_include_directories(deadlocked PRIVATE include)

target_include_directories(deadlocked PRIVATE imgui)
target_include_directories(deadlocked PRIVATE imgui/backends)
target_sources(deadlocked PRIVATE imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl3.cpp imgui/backends/imgui_impl_opengl3.cpp)

add_subdirectory(glm)
add_subdirectory(sdl)

target_link_libraries(deadlocked PRIVATE glm::glm)
target_link_libraries(deadlocked PRIVATE OpenGL)
target_link_libraries(deadlocked PRIVATE SDL3::SDL3)

if (CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(deadlocked PRIVATE "-g3" "-Wall" "-Wextra" "-Wconversion"
    "-Wdouble-promotion" "-Wno-unused-parameter" "-Wno-unused-function"
    "-Wno-sign-conversion")
endif()

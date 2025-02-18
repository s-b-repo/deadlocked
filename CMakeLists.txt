cmake_minimum_required(VERSION 3.28)
project(deadlocked CXX)

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "this only works on linux.")
endif()

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

add_executable(deadlocked ${SOURCE_FILES})
target_include_directories(deadlocked PRIVATE include)

target_include_directories(deadlocked PRIVATE lib/imgui)
target_include_directories(deadlocked PRIVATE lib/imgui/backends)
target_sources(deadlocked PRIVATE lib/imgui/imgui.cpp lib/imgui/imgui_demo.cpp lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_sdl3.cpp lib/imgui/backends/imgui_impl_opengl3.cpp)

set(OATPP_MODULES_LOCATION CUSTOM)
set(OATPP_DIR_SRC "${CMAKE_CURRENT_SOURCE_DIR}/lib/oatpp/src")
set(OATPP_DIR_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/oatpp/build/src")
set(OATPP_BUILD_TESTS OFF CACHE BOOL "disable oatpp tests" FORCE)
set(OATPP_BUILD_EXAMPLES OFF CACHE BOOL "disable oatpp examples" FORCE)

set(SDL_TESTS OFF CACHE BOOL "disable sdl tests" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "disable sdl examples" FORCE)
set(SDL_INSTALL_DOCS OFF CACHE BOOL "disable sdl docs" FORCE)

add_subdirectory(lib/glm)
add_subdirectory(lib/oatpp)
add_subdirectory(lib/oatpp-websocket)
add_subdirectory(lib/sdl)

target_link_libraries(deadlocked PRIVATE glm::glm)
target_link_libraries(deadlocked PRIVATE oatpp)
target_link_libraries(deadlocked PRIVATE oatpp-websocket)
target_link_libraries(deadlocked PRIVATE OpenGL)
target_link_libraries(deadlocked PRIVATE SDL3::SDL3)

if (CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_C_COMPILER_ID MATCHES "GNU")
    target_compile_options(deadlocked PRIVATE "-g3" "-Wall" "-Wextra" "-Wconversion"
    "-Wdouble-promotion" "-Wno-unused-parameter" "-Wno-unused-function"
    "-Wno-sign-conversion")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

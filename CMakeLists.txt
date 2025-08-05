cmake_minimum_required(VERSION 3.10)
project(MyProject)

# Debug configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Compiler flags for debugging
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra)
    add_compile_definitions(DEBUG_ENABLED=1)
    message(STATUS "Debug build enabled with full debugging symbols")
else()
    add_compile_options(-O2)
    add_compile_definitions(DEBUG_ENABLED=0)
    message(STATUS "Release build enabled with optimizations")
endif()

# Only include our specific source files
set(SOURCES sound.c)

add_executable(app ${SOURCES})

# FetchContent downloads and configures dependencies
include(FetchContent)

FetchContent_Declare(
    SDL2
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG "main"
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL_mixer.git"
    GIT_TAG "main"
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(SDL2_mixer)

# Link SDL3 targets
target_link_libraries(app PRIVATE SDL3::SDL3-shared)

# Link SDL3_mixer target
target_link_libraries(app PRIVATE SDL3_mixer::SDL3_mixer-shared m)
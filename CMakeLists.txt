cmake_minimum_required(VERSION 3.10)
project(SDL2_Project)

# Specify the executable name
set(EXECUTABLE_NAME "app")

# Locate SDL2 package
find_package(SDL2 REQUIRED)

# Include SDL2 headers
include_directories(${SDL2_INCLUDE_DIRS})

# Locate SDL2_mixer package
find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_MIXER_INCLUDE_DIRS})

# Add the main source file
add_executable(${EXECUTABLE_NAME} sound.c)

# Link SDL2 and SDL2_mixer libraries to the executable
target_link_libraries(${EXECUTABLE_NAME} ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES} m )

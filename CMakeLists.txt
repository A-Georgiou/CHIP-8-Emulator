cmake_minimum_required(VERSION 3.14)
project(chip-8)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Add the FetchContent module
include(FetchContent)

# Fetch and install Raylib
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.0 # You can specify the version here
)

FetchContent_MakeAvailable(raylib)

# Fetch and install Glad
FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG v0.1.36 # You can specify the version here
)

FetchContent_MakeAvailable(glad)

# Add executable
add_executable(chip-8
    src/main.cpp
    src/chip8.cpp
)

# Include directories
target_include_directories(chip-8 PRIVATE
    include 
    ${raylib_SOURCE_DIR}/src 
    ${glad_SOURCE_DIR}/include
    ${raylib_BINARY_DIR}
)

# Link libraries
target_link_libraries(chip-8 raylib glad)

# Platform-specific configurations
if (WIN32)
    target_link_libraries(chip-8 opengl32)
elseif (APPLE)
    target_link_libraries(chip-8 "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif (UNIX)
    target_link_libraries(chip-8 GL dl pthread)
endif()

# Enable compiler warnings
if(MSVC)
    target_compile_options(chip-8 PRIVATE /W4 /WX)
else()
    target_compile_options(chip-8 PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

cmake_minimum_required(VERSION 3.8)

project("SimpleBox")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(include)

# SFML from Git
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# Executable
add_executable(SimpleBox
    src/Main.cpp
    src/Game.cpp
    src/Materials.cpp
    src/UIScaler.cpp
)

# Link SFML
target_link_libraries(SimpleBox PRIVATE sfml-graphics)

# Copy resources to bin/resources
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_BINARY_DIR}/bin/resources
)

add_dependencies(SimpleBox copy_resources)

cmake_minimum_required(VERSION 3.16)
project(SimpleBox)

# Set the standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# External SFML library
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

# Include directories
include_directories(include)

# Collecting all .cpp files
file(GLOB SOURCES "src/*.cpp")

# Create an executable file
add_executable(SimpleBox ${SOURCES})

# Linking SFML
target_link_libraries(SimpleBox sfml-graphics sfml-window sfml-system)
cmake_minimum_required(VERSION 3.12)
project(usr_canet_cpp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags for release build
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Find Python and pybind11
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Python3_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/usr_canet_bus.cpp
    src/bindings.cpp
)

# Create Python extension module
pybind11_add_module(_usr_canet_cpp ${SOURCES})

# Link libraries
target_link_libraries(_usr_canet_cpp PRIVATE
    pthread
)

# Installation
install(TARGETS _usr_canet_cpp
    LIBRARY DESTINATION .
)

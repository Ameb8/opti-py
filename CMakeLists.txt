cmake_minimum_required(VERSION 3.15)
project(opti_py LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (default Release)" FORCE)
endif()

find_package(pybind11 REQUIRED)

# Include header files
include_directories(src/opti_py/cpp/include/opti_py)

# Collect all cpp files
file(GLOB_RECURSE CPP_SOURCES
    src/opti_py/cpp/src/*.cpp
)

# Add python bindings
list(APPEND CPP_SOURCES src/opti_py/cpp/bindings.cpp)

# Build the Python module
pybind11_add_module(_opti_py ${CPP_SOURCES})

# CPP compiler flags
target_compile_options(_opti_py PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>
    $<$<CONFIG:Release>:-O3>
)


# Install module
install(TARGETS _opti_py
        LIBRARY DESTINATION opti_py)

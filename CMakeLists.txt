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

# Find OpenMP on MacOS
if(APPLE)
    execute_process(
        COMMAND brew --prefix libomp
        OUTPUT_VARIABLE OpenMP_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(OpenMP_ROOT)
        set(OpenMP_CXX_FLAGS "-fopenmp=libomp")
        set(OpenMP_CXX_LIB_NAMES libomp)
        set(OpenMP_libomp_LIBRARY ${OpenMP_ROOT}/lib/libomp.dylib)
    endif()
endif()

# Require OpenMP
find_package(OpenMP REQUIRED)


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

# Link OpenMP
if(OpenMP_CXX_FOUND)
    target_link_libraries(_opti_py PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(_opti_py PRIVATE OPTI_PY_OPENMP)
endif()


# CPP compiler flags
target_compile_options(_opti_py PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>
    $<$<CONFIG:Release>:-O3>
)


# Install module
install(TARGETS _opti_py
        LIBRARY DESTINATION opti_py)

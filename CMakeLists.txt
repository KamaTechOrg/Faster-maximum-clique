# LazyMC - Lazy Maximum Clique Algorithm
cmake_minimum_required(VERSION 3.14)
project(LazyMC VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_CLI "Build CLI application" ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all source files for LazyMC library
file(GLOB_RECURSE LAZYMC_SOURCES
    "src/graph/*.cpp"
    "src/core/*.cpp"
    "src/heuristics/*.cpp"
    "src/search/*.cpp"
    "src/lazymc.cpp"
)

# Create LazyMC library
add_library(lazymc STATIC ${LAZYMC_SOURCES})
target_include_directories(lazymc PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Build CLI application
if(BUILD_CLI)
    add_executable(lazymc_cli apps/lazymc_cli.cpp)
    target_link_libraries(lazymc_cli PRIVATE lazymc)
endif()

# Load FetchContent for external dependencies
include(FetchContent)

# Keep MCQD baseline (optional, for comparison)
FetchContent_Declare(
    mcqd
    GIT_REPOSITORY https://gitlab.com/janezkonc/mcqd.git
    GIT_TAG        9ab9717cf7d1be1a85b165a8eacb71b9e5831113
)

FetchContent_GetProperties(mcqd)
if(NOT mcqd_POPULATED)
    FetchContent_Populate(mcqd)
endif()

# MCQD library (optional - may have compilation issues on Windows)
add_library(mcqd_lib STATIC EXCLUDE_FROM_ALL
    ${mcqd_SOURCE_DIR}/mcqd.cpp
)
target_include_directories(mcqd_lib PUBLIC
    ${mcqd_SOURCE_DIR}
)

# Old main.cpp (baseline comparison)
add_executable(my_benchmark_runner EXCLUDE_FROM_ALL main.cpp)
target_link_libraries(my_benchmark_runner PRIVATE mcqd_lib)

# Enable testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS lazymc
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/lazymc
    DESTINATION include
)

if(BUILD_CLI)
    install(TARGETS lazymc_cli
        RUNTIME DESTINATION bin
    )
endif()

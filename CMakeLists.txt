# Requires CMake version 3.14 or later for FetchContent
cmake_minimum_required(VERSION 3.14)
project(MyCliqueBenchmark LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for algorithms
include_directories(${CMAKE_SOURCE_DIR}/algorithms)

# 1. Define the main executable (we'll call it my_benchmark_runner)
add_executable(my_benchmark_runner main.cpp)

# 2. Load the FetchContent module
include(FetchContent)

# 3. Declare the dependency (MaxCliqueDyn)
# GitLab URL: https://gitlab.com/janezkonc/mcqd.git [4]
FetchContent_Declare(
    mcqd
    GIT_REPOSITORY https://gitlab.com/janezkonc/mcqd.git
    GIT_TAG        9ab9717cf7d1be1a85b165a8eacb71b9e5831113
)

# 4. Download the source code into the build directory (Populate)
FetchContent_GetProperties(mcqd)
if(NOT mcqd_POPULATED)
    FetchContent_Populate(mcqd)
endif()

# 5. Create a static library target for MCQD
# MCQD does not contain a CMakeLists.txt, therefore we manually create a (wrapper) target
# that links to the mcqd.cpp source file that was downloaded.
add_library(mcqd_lib STATIC
    ${mcqd_SOURCE_DIR}/mcqd.cpp
)

# 6. Set the include directory
target_include_directories(mcqd_lib PUBLIC
    ${mcqd_SOURCE_DIR}
)

# 7. Link the library to the main executable
target_link_libraries(my_benchmark_runner PRIVATE mcqd_lib)

# 8. Enable testing
enable_testing()

# 9. Add tests subdirectory
add_subdirectory(tests)

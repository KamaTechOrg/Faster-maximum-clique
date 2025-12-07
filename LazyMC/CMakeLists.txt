cmake_minimum_required(VERSION 3.20)

# Use old behavior for FetchContent_Populate(mcqd) and silence CMP0169 warning
if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()
project(LazyMC LANGUAGES CXX)

# ---- Options ----
option(BUILD_TESTING "Build tests" ON)

# ---- Toolchain ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---- External: MCQD (for baseline max-clique) ----
include(FetchContent)

FetchContent_Declare(
  mcqd
  GIT_REPOSITORY https://gitlab.com/janezkonc/mcqd.git
  GIT_TAG        9ab9717cf7d1be1a85b165a8eacb71b9e5831113
)

FetchContent_GetProperties(mcqd)
if(NOT mcqd_POPULATED)
  FetchContent_Populate(mcqd)
endif()

add_library(mcqd_lib STATIC
  ${mcqd_SOURCE_DIR}/mcqd.cpp
)

target_include_directories(mcqd_lib
  PUBLIC
    ${mcqd_SOURCE_DIR}
)

# ---- Library ----
add_library(lazymc STATIC
  src/bk.cpp
  src/corness_based_heuristic.cpp
  src/degree_based_heuristic.cpp
  src/intersect_functions.cpp
  src/k_vertex_cover.cpp
  src/lazygraph.cpp
  src/lazymc.cpp
  src/NeighborSearch.cpp
  src/Systematic_search.cpp
  src/graph_processing.cpp
  
)

target_include_directories(lazymc
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---- CLI ----
add_executable(lazymc_cli src/main.cpp)
target_link_libraries(lazymc_cli PRIVATE lazymc)

# ---- Benchmarks ----
add_executable(bk_cli src/bk_cli.cpp)
target_link_libraries(bk_cli PRIVATE lazymc)

# ---- Vertex Cover CLI ----
add_executable(vc_cli src/vc_cli.cpp)
target_link_libraries(vc_cli PRIVATE lazymc)


# ---- Testing (GoogleTest via FetchContent) ----
if(BUILD_TESTING)
  include(CTest)
  enable_testing()

  include(FetchContent)

  # Declare GoogleTest as an external dependency (download at configure time)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2  # use a stable release tag
  )

  # On Windows, make sure we use the same runtime library
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Download and add GoogleTest targets (GTest::gtest, GTest::gtest_main, etc.)
  FetchContent_MakeAvailable(googletest)

  # Tests are defined in the tests/ subdirectory
  add_subdirectory(tests)
endif()

cmake_minimum_required(VERSION 3.20)
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

# ---- Library ----
add_library(lazymc STATIC
  src/bk.cpp
  src/coreness_based_heuristic.cpp
  src/degree_based_heuristic.cpp
  src/intersect_functions.cpp
  src/k_vertex_cover.cpp
  src/lazygraph.cpp
  src/lazymc.cpp
  src/neighbor_search.cpp
  src/systematic_search.cpp
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
add_executable(bk_bench src/bk_bench.cpp)
target_link_libraries(bk_bench PRIVATE lazymc)

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

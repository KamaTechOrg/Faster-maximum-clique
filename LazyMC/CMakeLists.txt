cmake_minimum_required(VERSION 3.20)

# Use old behavior for FetchContent_Populate(...) and silence CMP0169 warning (if present)
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

# ---- External: PMC (baseline max-clique) ----
include(FetchContent)

FetchContent_Declare(
  pmc
  GIT_REPOSITORY https://github.com/ryanrossi/pmc.git
  GIT_TAG        06594a0
)

FetchContent_GetProperties(pmc)
if(NOT pmc_POPULATED)
  FetchContent_Populate(pmc)
endif()

# Patch PMC for modern libstdc++ (g++ 15): disallow implicit ifstream -> bool conversion
set(PMC_UTILS_CPP "${pmc_SOURCE_DIR}/pmc_utils.cpp")
if(EXISTS "${PMC_UTILS_CPP}")
  file(READ "${PMC_UTILS_CPP}" PMC_UTILS_TXT)

  # Only patch if the old line exists (idempotent)
  if(PMC_UTILS_TXT MATCHES "return[ \t]+ifile;")
    string(REPLACE
      "return ifile;"
      "return static_cast<bool>(ifile);"
      PMC_UTILS_TXT
      "${PMC_UTILS_TXT}"
    )
    file(WRITE "${PMC_UTILS_CPP}" "${PMC_UTILS_TXT}")
  endif()
else()
  message(FATAL_ERROR "PMC file not found for patching: ${PMC_UTILS_CPP}")
endif()

add_library(pmc_lib STATIC
  ${pmc_SOURCE_DIR}/pmc_utils.cpp
  ${pmc_SOURCE_DIR}/pmc_graph.cpp
  ${pmc_SOURCE_DIR}/pmc_clique_utils.cpp
  ${pmc_SOURCE_DIR}/pmc_heu.cpp
  ${pmc_SOURCE_DIR}/pmc_maxclique.cpp
  ${pmc_SOURCE_DIR}/pmcx_maxclique.cpp
  ${pmc_SOURCE_DIR}/pmcx_maxclique_basic.cpp
  ${pmc_SOURCE_DIR}/pmc_cores.cpp
  ${pmc_SOURCE_DIR}/pmc_lib.cpp
)

target_include_directories(pmc_lib PUBLIC ${pmc_SOURCE_DIR})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  target_link_libraries(pmc_lib PUBLIC OpenMP::OpenMP_CXX)
endif()

# ---- Library ----
add_library(lazymc STATIC
  src/bk.cpp
  src/bk_bitset.cpp
  src/degree_based_heuristic.cpp
  src/intersect_functions.cpp
  src/k_vertex_cover.cpp
  src/lazygraph.cpp
  src/lazymc.cpp
  src/neighbor_search.cpp
  src/systematic_search.cpp
  src/graph_processing.cpp
  src/kcore.cpp
  src/determine_sort_order.cpp
  src/coreness_heuristic.cpp
)

target_include_directories(lazymc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---- CLI ----
add_executable(lazymc_cli src/main.cpp)
target_link_libraries(lazymc_cli PRIVATE lazymc)

add_executable(bench_cli
  src/bench_cli.cpp
  src/pmc_adapter.cpp
)
target_link_libraries(bench_cli PRIVATE lazymc pmc_lib)

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

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
  )

  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  add_subdirectory(tests)
endif()

# CMakeLists.txt for tests directory

add_executable(lazymc_tests
  test_smoke.cpp
  test_graph_processing.cpp
  test_bk.cpp
  test_NeighborSearch.cpp
  test_k_vertex_cover.cpp
  test_degree_heuristic.cpp
  test_intersect_functions.cpp
)

target_link_libraries(lazymc_tests
  PRIVATE
    lazymc
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lazymc_tests)

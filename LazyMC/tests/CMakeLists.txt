# CMakeLists.txt for tests directory

add_executable(lazymc_tests
  test_smoke.cpp
  test_graph_processing.cpp
  test_bk.cpp
  test_neighbor_search.cpp
  test_k_vertex_cover.cpp
  test_degree_heuristic.cpp
  test_intersect_functions.cpp
  test_lazygraph.cpp
  test_kcore.cpp
  test_determine_sort_order.cpp
  test_algo1.cpp
  test_coreness_heuristic.cpp
  test_systematic_search.cpp
  test_lazymc_correctness_small_random.cpp

)

target_link_libraries(lazymc_tests
  PRIVATE
    lazymc
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(lazymc_tests)

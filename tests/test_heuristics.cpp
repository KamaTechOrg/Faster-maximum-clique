#include <gtest/gtest.h>
#include "lazymc/heuristics/degree_heuristic.hpp"
#include "lazymc/heuristics/coreness_heuristic.hpp"

namespace {

using namespace lazymc::heuristics;

TEST(HeuristicsTest, DegreeHeuristic) {
    std::vector<std::vector<int>> adj = {
        {1, 2, 3},
        {0, 2, 3},
        {0, 1, 3},
        {0, 1, 2}
    };
    
    auto top_k = select_top_k_by_degree(adj, 2);
    
    ASSERT_EQ(top_k.size(), 2);
    // All vertices have degree 3, so first two by index
    EXPECT_EQ(top_k[0], 0);
    EXPECT_EQ(top_k[1], 1);
}

TEST(HeuristicsTest, EstimateUpperBound) {
    std::vector<std::vector<int>> adj = {
        {1, 2},
        {0, 2},
        {0, 1}
    };
    
    int bound = CorenessHeuristic::estimate_upper_bound(adj);
    
    EXPECT_GT(bound, 0);
}

} // namespace

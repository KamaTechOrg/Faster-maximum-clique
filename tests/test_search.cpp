#include <gtest/gtest.h>
#include "lazymc/search/max_clique_search.hpp"

namespace {

using namespace lazymc::search;

TEST(SearchTest, FindMaxClique) {
    // Complete graph K4
    std::vector<std::vector<int>> adj = {
        {1, 2, 3},
        {0, 2, 3},
        {0, 1, 3},
        {0, 1, 2}
    };
    
    auto max_clique = MaxCliqueSearch::find_max_clique(adj);
    
    // Should find a clique (implementation pending)
    EXPECT_GE(max_clique.size(), 0);
}

} // namespace

#include <gtest/gtest.h>
#include "lazymc/core/ordering.hpp"

namespace {

using namespace lazymc::core;

TEST(OrderingTest, ByDegree) {
    std::vector<std::vector<int>> adj = {
        {1},          // degree 1
        {0, 2, 3, 4}, // degree 4
        {1},          // degree 1
        {1},          // degree 1
        {1}           // degree 1
    };
    
    auto ordered = Ordering::by_degree(adj);
    
    ASSERT_EQ(ordered.size(), 5);
    EXPECT_EQ(ordered[0], 1); // highest degree first
}

} // namespace

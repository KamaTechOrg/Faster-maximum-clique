#include <gtest/gtest.h>
#include "algorithm5/helper_functions/top_k_by_degree.hpp"

namespace {

// Test basic functionality
TEST(TopKByDegreeTest, BasicSelection) {
    // Create a simple graph with 5 vertices
    std::vector<std::vector<int>> adj = {
        {1, 2},       // vertex 0: degree 2
        {0, 2, 3, 4}, // vertex 1: degree 4
        {0, 1, 3},    // vertex 2: degree 3
        {1, 2, 4},    // vertex 3: degree 3
        {1, 3}        // vertex 4: degree 2
    };

    auto result = graph::select_top_k_by_degree(adj, 3);
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 1); // vertex 1 has degree 4 (highest)
    // vertices 2 and 3 both have degree 3, but 2 comes first
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
}

// Test with K larger than graph size
TEST(TopKByDegreeTest, KLargerThanN) {
    std::vector<std::vector<int>> adj = {
        {1},
        {0, 2},
        {1}
    };

    auto result = graph::select_top_k_by_degree(adj, 10);
    
    ASSERT_EQ(result.size(), 3); // Should return all 3 vertices
}

// Test with empty graph
TEST(TopKByDegreeTest, EmptyGraph) {
    std::vector<std::vector<int>> adj;

    auto result = graph::select_top_k_by_degree(adj, 5);
    
    EXPECT_TRUE(result.empty());
}

// Test with K = 0
TEST(TopKByDegreeTest, ZeroK) {
    std::vector<std::vector<int>> adj = {
        {1, 2},
        {0, 2},
        {0, 1}
    };

    auto result = graph::select_top_k_by_degree(adj, 0);
    
    EXPECT_TRUE(result.empty());
}

// Test with single vertex
TEST(TopKByDegreeTest, SingleVertex) {
    std::vector<std::vector<int>> adj = {
        {}
    };

    auto result = graph::select_top_k_by_degree(adj, 1);
    
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0);
}

// Test tie-breaking by vertex index
TEST(TopKByDegreeTest, TieBreaking) {
    std::vector<std::vector<int>> adj = {
        {3},     // vertex 0: degree 1
        {3},     // vertex 1: degree 1
        {3},     // vertex 2: degree 1
        {0, 1, 2}// vertex 3: degree 3
    };

    auto result = graph::select_top_k_by_degree(adj, 3);
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 3); // vertex 3 has highest degree
    // Among vertices with degree 1, lower indices come first
    EXPECT_EQ(result[1], 0);
    EXPECT_EQ(result[2], 1);
}

} // namespace

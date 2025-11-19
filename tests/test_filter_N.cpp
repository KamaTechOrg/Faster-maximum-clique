#include <gtest/gtest.h>
#include "lazymc/core/candidate_ops.hpp"
#include "lazymc/graph/graph.hpp"

namespace {

using namespace lazymc;
using namespace lazymc::graph;

// Test fixture for filter_N tests
class FilterNTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a simple graph for testing
        // Graph structure:
        //     0 --- 1
        //     |   / |
        //     | /   |
        //     2 --- 3
        //       \   |
        //         \ |
        //           4
        
        g = Graph(5);
        g.add_edge(0, 1);
        g.add_edge(0, 2);
        g.add_edge(1, 2);
        g.add_edge(1, 3);
        g.add_edge(2, 3);
        g.add_edge(2, 4);
        g.add_edge(3, 4);
    }

    Graph g;
};

TEST_F(FilterNTest, BasicFiltering) {
    // N = {0, 1, 2, 3}
    // u = 1 (neighbors: 0, 2, 3)
    // Expected: N ← {0, 2, 3}
    
    std::vector<int> N = {0, 1, 2, 3};
    std::vector<char> inN(5, 0);
    for (int v : N) inN[v] = 1;
    
    filter_N(N, inN, 1, g);
    
    // Check N contains only neighbors of u=1
    ASSERT_EQ(N.size(), 3);
    EXPECT_TRUE(std::find(N.begin(), N.end(), 0) != N.end());
    EXPECT_TRUE(std::find(N.begin(), N.end(), 2) != N.end());
    EXPECT_TRUE(std::find(N.begin(), N.end(), 3) != N.end());
    EXPECT_FALSE(std::find(N.begin(), N.end(), 1) != N.end());
    
    // Check inN is updated correctly
    EXPECT_EQ(inN[0], 1);
    EXPECT_EQ(inN[1], 0);
    EXPECT_EQ(inN[2], 1);
    EXPECT_EQ(inN[3], 1);
    EXPECT_EQ(inN[4], 0);
}

TEST_F(FilterNTest, FilterWithNoCommonNeighbors) {
    // N = {4}
    // u = 0 (neighbors: 1, 2)
    // Expected: N ← {} (empty)
    
    std::vector<int> N = {4};
    std::vector<char> inN(5, 0);
    inN[4] = 1;
    
    filter_N(N, inN, 0, g);
    
    EXPECT_TRUE(N.empty());
    EXPECT_EQ(inN[4], 0);
}

TEST_F(FilterNTest, FilterWithAllCommonNeighbors) {
    // N = {0, 2}
    // u = 1 (neighbors: 0, 2, 3)
    // Expected: N ← {0, 2} (all remain)
    
    std::vector<int> N = {0, 2};
    std::vector<char> inN(5, 0);
    inN[0] = 1;
    inN[2] = 1;
    
    filter_N(N, inN, 1, g);
    
    ASSERT_EQ(N.size(), 2);
    EXPECT_TRUE(std::find(N.begin(), N.end(), 0) != N.end());
    EXPECT_TRUE(std::find(N.begin(), N.end(), 2) != N.end());
}

TEST_F(FilterNTest, FilterEmptyN) {
    // N = {}
    // Expected: N ← {} (stays empty)
    
    std::vector<int> N;
    std::vector<char> inN(5, 0);
    
    filter_N(N, inN, 1, g);
    
    EXPECT_TRUE(N.empty());
}

TEST_F(FilterNTest, FilterComplexCase) {
    // N = {1, 2, 3, 4}
    // u = 2 (neighbors: 0, 1, 3, 4)
    // Expected: N ← {1, 3, 4}
    
    std::vector<int> N = {1, 2, 3, 4};
    std::vector<char> inN(5, 0);
    for (int v : N) inN[v] = 1;
    
    filter_N(N, inN, 2, g);
    
    ASSERT_EQ(N.size(), 3);
    EXPECT_TRUE(std::find(N.begin(), N.end(), 1) != N.end());
    EXPECT_TRUE(std::find(N.begin(), N.end(), 3) != N.end());
    EXPECT_TRUE(std::find(N.begin(), N.end(), 4) != N.end());
    EXPECT_FALSE(std::find(N.begin(), N.end(), 2) != N.end());
    
    EXPECT_EQ(inN[1], 1);
    EXPECT_EQ(inN[2], 0);
    EXPECT_EQ(inN[3], 1);
    EXPECT_EQ(inN[4], 1);
}

// Tests for rebuild_inN
TEST(RebuildInNTest, BasicRebuild) {
    std::vector<int> N = {0, 2, 4};
    std::vector<char> inN(6, 0);
    
    rebuild_inN(inN, N);
    
    EXPECT_EQ(inN[0], 1);
    EXPECT_EQ(inN[1], 0);
    EXPECT_EQ(inN[2], 1);
    EXPECT_EQ(inN[3], 0);
    EXPECT_EQ(inN[4], 1);
    EXPECT_EQ(inN[5], 0);
}

TEST(RebuildInNTest, EmptyN) {
    std::vector<int> N;
    std::vector<char> inN(5, 1); // Start with all 1s
    
    rebuild_inN(inN, N);
    
    // All should be 0
    for (char val : inN) {
        EXPECT_EQ(val, 0);
    }
}

TEST(RebuildInNTest, RebuildAfterModification) {
    std::vector<char> inN(5, 1); // Start with all 1s
    std::vector<int> N = {1, 3};
    
    rebuild_inN(inN, N);
    
    EXPECT_EQ(inN[0], 0);
    EXPECT_EQ(inN[1], 1);
    EXPECT_EQ(inN[2], 0);
    EXPECT_EQ(inN[3], 1);
    EXPECT_EQ(inN[4], 0);
}

TEST(RebuildInNTest, AllVertices) {
    std::vector<int> N = {0, 1, 2, 3, 4};
    std::vector<char> inN(5, 0);
    
    rebuild_inN(inN, N);
    
    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(inN[i], 1);
    }
}

TEST(RebuildInNTest, BoundaryCheck) {
    // N contains valid indices
    std::vector<int> N = {0, 4};
    std::vector<char> inN(5, 0);
    
    rebuild_inN(inN, N);
    
    EXPECT_EQ(inN[0], 1);
    EXPECT_EQ(inN[4], 1);
}

} // namespace

#include <gtest/gtest.h>
#include "lazymc/lazygraph.h"
#include <algorithm>

using lazymc::AdjList;
using lazymc::LazyGraph;

// ------------------------------------------------------------
// Test 1: Basic construction and simple mapping
// ------------------------------------------------------------

TEST(LazyGraphBasic, ConstructionAndAccessors) {
    // Simple original graph:
    // 0: 1,2
    // 1: 0
    // 2: 0
    AdjList G = {
        {1, 2}, // 0
        {0},    // 1
        {0}     // 2
    };

    // Identity-like mapping:
    // H = G (0 -> 0, 1 -> 1, 2 -> 2)
    std::vector<int32_t> r2o = {0, 1, 2}; // H -> G
    std::vector<int32_t> o2r = {0, 1, 2}; // G -> H

    // Coreness in H-labeling
    std::vector<int32_t> core = {3, 2, 1};

    LazyGraph H(G, r2o, o2r, core);

    // Basic size and graph pointer
    EXPECT_EQ(H.num_vertices(), 3u);
    EXPECT_EQ(&H.original_graph(), &G);

    // Mapping H -> G
    EXPECT_EQ(H.to_original(0), 0);
    EXPECT_EQ(H.to_original(1), 1);
    EXPECT_EQ(H.to_original(2), 2);

    // Mapping G -> H
    EXPECT_EQ(H.to_relabelled(0), 0);
    EXPECT_EQ(H.to_relabelled(1), 1);
    EXPECT_EQ(H.to_relabelled(2), 2);

    // Coreness preserved
    const auto& core_vec = H.core_values();
    ASSERT_EQ(core_vec.size(), 3u);
    EXPECT_EQ(core_vec[0], 3);
    EXPECT_EQ(core_vec[1], 2);
    EXPECT_EQ(core_vec[2], 1);

    // Expected neighborhoods in H-labeling:
    // Using full neighbor lists (no filtering):
    //
    // v = 0: neighbors -> {1,2}
    // v = 1: neighbors -> {0}
    // v = 2: neighbors -> {0}
    std::vector<std::vector<int32_t>> expected = {
        {1, 2}, // H=0
        {0},    // H=1
        {0}     // H=2
    };

    for (int32_t v = 0; v < 3; ++v) {
        auto& hv = H.hashed_neighbors(v);
        auto const& sv = H.sorted_neighbors(v);

        // Check hashed neighbors as a set
        EXPECT_EQ(hv.size(), expected[v].size());
        for (int32_t u : expected[v]) {
            EXPECT_EQ(hv.count(u), 1u);
        }

        // Check sorted neighbors as a sorted vector
        ASSERT_EQ(sv.size(), expected[v].size());
        for (std::size_t i = 0; i < sv.size(); ++i) {
            EXPECT_EQ(sv[i], expected[v][i]);
        }
    }
}

// ------------------------------------------------------------
// Test 2: Non-trivial permutation and per-vertex containers
// ------------------------------------------------------------

TEST(LazyGraphBasic, PermutationMappingAndContainers) {
    // Original graph G:
    //
    // 0: 1,2
    // 1: 0,3
    // 2: 0,3,4
    // 3: 1,2
    // 4: 2
    AdjList G = {
        {1, 2},      // 0
        {0, 3},      // 1
        {0, 3, 4},   // 2
        {1, 2},      // 3
        {2}          // 4
    };

    // Reordering (H-labeling):
    //
    // H: 0  1  2  3  4
    // G: 2  0  3  4  1
    //
    // So:
    //   H=0 -> G=2
    //   H=1 -> G=0
    //   H=2 -> G=3
    //   H=3 -> G=4
    //   H=4 -> G=1
    std::vector<int32_t> r2o = {2, 0, 3, 4, 1}; // H -> G
    std::vector<int32_t> o2r = {1, 4, 0, 2, 3}; // G -> H

    // Coreness in H-labeling (arbitrary, but consistent)
    std::vector<int32_t> core = {
        5, // H=0
        4, // H=1
        3, // H=2
        2, // H=3
        1  // H=4
    };

    LazyGraph H(G, r2o, o2r, core);

    // Size and original graph pointer
    EXPECT_EQ(H.num_vertices(), 5u);
    EXPECT_EQ(&H.original_graph(), &G);

    // H -> G mapping
    EXPECT_EQ(H.to_original(0), 2);
    EXPECT_EQ(H.to_original(1), 0);
    EXPECT_EQ(H.to_original(2), 3);
    EXPECT_EQ(H.to_original(3), 4);
    EXPECT_EQ(H.to_original(4), 1);

    // G -> H mapping
    EXPECT_EQ(H.to_relabelled(0), 1);
    EXPECT_EQ(H.to_relabelled(1), 4);
    EXPECT_EQ(H.to_relabelled(2), 0);
    EXPECT_EQ(H.to_relabelled(3), 2);
    EXPECT_EQ(H.to_relabelled(4), 3);

    // Coreness preserved
    const auto& core_vec = H.core_values();
    ASSERT_EQ(core_vec.size(), 5u);
    EXPECT_EQ(core_vec[0], 5);
    EXPECT_EQ(core_vec[1], 4);
    EXPECT_EQ(core_vec[2], 3);
    EXPECT_EQ(core_vec[3], 2);
    EXPECT_EQ(core_vec[4], 1);

    // Expected neighborhoods in H-labeling
    //
    // For each H vertex, convert neighbors from G using o2r:
    //
    // H=0 -> G=2, N_G(2) = {0,3,4} -> H = {1,2,3}
    // H=1 -> G=0, N_G(0) = {1,2}   -> H = {4,0} => sorted: {0,4}
    // H=2 -> G=3, N_G(3) = {1,2}   -> H = {4,0} => sorted: {0,4}
    // H=3 -> G=4, N_G(4) = {2}     -> H = {0}
    // H=4 -> G=1, N_G(1) = {0,3}   -> H = {1,2}
    std::vector<std::vector<int32_t>> expected = {
        {1, 2, 3}, // H=0
        {0, 4},    // H=1
        {0, 4},    // H=2
        {0},       // H=3
        {1, 2}     // H=4
    };

    // First: verify that hashed and sorted neighbors match expectations
    std::vector<std::size_t> size_before(5, 0);

    for (int32_t v = 0; v < 5; ++v) {
        auto& hv = H.hashed_neighbors(v);
        auto const& sv = H.sorted_neighbors(v);

        // Check hashed as a set
        EXPECT_EQ(hv.size(), expected[v].size());
        for (int32_t u : expected[v]) {
            EXPECT_EQ(hv.count(u), 1u);
        }

        // Check sorted as a sorted vector
        ASSERT_EQ(sv.size(), expected[v].size());
        for (std::size_t i = 0; i < sv.size(); ++i) {
            EXPECT_EQ(sv[i], expected[v][i]);
        }

        // Remember current size of hashed neighbors
        size_before[static_cast<std::size_t>(v)] = hv.size();
    }

    // Now insert a unique marker into hashed neighbors for each vertex
    for (int32_t v = 0; v < 5; ++v) {
        auto& hv = H.hashed_neighbors(v);
        hv.insert(100 + v);
    }

    // Verify that each vertex has its own hashed container and markers do not mix
    for (int32_t v = 0; v < 5; ++v) {
        const auto& hv = H.hashed_neighbors(v);
        const std::size_t base_size = size_before[static_cast<std::size_t>(v)];

        // Exactly one extra element per vertex compared to original neighbors
        EXPECT_EQ(hv.size(), base_size + 1);

        // Marker for this vertex must exist
        EXPECT_EQ(hv.count(100 + v), 1u);

        // No markers from other vertices
        for (int32_t u = 0; u < 5; ++u) {
            if (u == v) continue;
            EXPECT_EQ(hv.count(100 + u), 0u);
        }
    }
}

// ------------------------------------------------------------
// Test 3: Isolated vertex and core_only checks
// ------------------------------------------------------------

TEST(LazyGraphBasic, IsolatedVertexAndCoreOnly) {
    // Graph with one isolated vertex:
    // 0: -
    AdjList G = {
        {} // 0
    };

    std::vector<int32_t> r2o = {0}; // H -> G
    std::vector<int32_t> o2r = {0}; // G -> H
    std::vector<int32_t> core = {7}; // arbitrary value

    LazyGraph H(G, r2o, o2r, core);

    // Basic info
    EXPECT_EQ(H.num_vertices(), 1u);
    EXPECT_EQ(&H.original_graph(), &G);
    EXPECT_EQ(H.to_original(0), 0);
    EXPECT_EQ(H.to_relabelled(0), 0);

    const auto& core_vec = H.core_values();
    ASSERT_EQ(core_vec.size(), 1u);
    EXPECT_EQ(core_vec[0], 7);

    // Both hashed and sorted neighbors of the isolated vertex must be empty
    auto& hv = H.hashed_neighbors(0);
    auto const& sv = H.sorted_neighbors(0);

    EXPECT_TRUE(hv.empty());
    EXPECT_TRUE(sv.empty());
}

// ------------------------------------------------------------
// Test 4: Lazy construction – sorted triggers hashed build
// ------------------------------------------------------------

TEST(LazyGraphBasic, SortedBuildsFromHashedLazy) {
    // Simple nontrivial graph
    // 0: 1
    // 1: 0,2
    // 2: 1
    AdjList G = {
        {1},    // 0
        {0, 2}, // 1
        {1}     // 2
    };

    std::vector<int32_t> r2o = {0, 1, 2}; // identity mapping
    std::vector<int32_t> o2r = {0, 1, 2};
    std::vector<int32_t> core = {1, 2, 3};

    LazyGraph H(G, r2o, o2r, core);

    // Call sorted_neighbors first for vertex 1
    auto const& sv = H.sorted_neighbors(1);

    // Expected neighbors of 1 in H = {0,2}, sorted as {0,2}
    ASSERT_EQ(sv.size(), 2u);
    EXPECT_EQ(sv[0], 0);
    EXPECT_EQ(sv[1], 2);

    // Now call hashed_neighbors and verify it is consistent
    auto& hv = H.hashed_neighbors(1);
    EXPECT_EQ(hv.size(), 2u);
    EXPECT_EQ(hv.count(0), 1u);
    EXPECT_EQ(hv.count(2), 1u);
}

// ------------------------------------------------------------
// Test 5: Stability under multiple calls
// ------------------------------------------------------------

TEST(LazyGraphBasic, MultipleCallsStability) {
    // Simple triangle:
    // 0: 1,2
    // 1: 0,2
    // 2: 0,1
    AdjList G = {
        {1, 2},
        {0, 2},
        {0, 1}
    };

    std::vector<int32_t> r2o = {0, 1, 2};
    std::vector<int32_t> o2r = {0, 1, 2};
    std::vector<int32_t> core = {3, 3, 3};

    LazyGraph H(G, r2o, o2r, core);

    // Call hashed_neighbors and sorted_neighbors multiple times
    for (int repeat = 0; repeat < 3; ++repeat) {
        for (int32_t v = 0; v < 3; ++v) {
            auto& hv = H.hashed_neighbors(v);
            auto const& sv = H.sorted_neighbors(v);

            // Triangle: each vertex has exactly 2 neighbors
            EXPECT_EQ(hv.size(), 2u);
            EXPECT_EQ(sv.size(), 2u);

            // Neighbors must be the other two vertices
            EXPECT_EQ(hv.count((v + 1) % 3), 1u);
            EXPECT_EQ(hv.count((v + 2) % 3), 1u);

            // Sorted neighbors are in ascending order
            std::vector<int32_t> expected = { (v + 1) % 3, (v + 2) % 3 };
            std::sort(expected.begin(), expected.end());

            std::vector<int32_t> actual = sv;
            std::sort(actual.begin(), actual.end());

            EXPECT_EQ(actual[0], expected[0]);
            EXPECT_EQ(actual[1], expected[1]);
        }
    }
}

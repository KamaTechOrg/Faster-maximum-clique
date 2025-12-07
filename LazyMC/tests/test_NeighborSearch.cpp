#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

// Forward declaration of NeighborSearch in the lazymc namespace.
namespace lazymc {

void NeighborSearch(const std::vector<std::vector<int32_t>>& G,
                    int32_t v,
                    const std::vector<int32_t>& core,
                    const std::vector<int32_t>& reorder,
                    int32_t& best_clique_size,
                    double phi);

} // namespace lazymc

using lazymc::NeighborSearch;

namespace {

using AdjList = std::vector<std::vector<int32_t>>;

// Helper: build an undirected edge between u and v.
void add_undirected_edge(AdjList& G, int32_t u, int32_t v) {
    G[static_cast<std::size_t>(u)].push_back(v);
    G[static_cast<std::size_t>(v)].push_back(u);
}

} // namespace


// ---------------------------------------------------------------------------
// Filter 1 tests
// ---------------------------------------------------------------------------

// Filter 1: N is built from neighbors with reorder[u] > reorder[v] && core[u] >= best_clique_size.
// This test checks the case where no neighbor qualifies, so N is empty and the function returns early.
TEST(NeighborSearchFilter1, NoQualifiedNeighborsReturnsEarly) {
    AdjList G(2);
    add_undirected_edge(G, 0, 1);

    // core[1] is too small to qualify
    std::vector<int32_t> core    = {1, 1};
    std::vector<int32_t> reorder = {0, 1};

    int32_t best_clique_size = 2;
    double phi = 0.5;

    NeighborSearch(G, /*v=*/0, core, reorder, best_clique_size, phi);

    // No change expected because Filter 1 prunes immediately.
    EXPECT_EQ(best_clique_size, 2);
}

// Filter 1: N has some neighbors but |N| < best_clique_size, so search is pruned.
TEST(NeighborSearchFilter1, NTooSmallPrunesSearch) {
    // Path 0 - 1 - 2
    AdjList G(3);
    add_undirected_edge(G, 0, 1);
    add_undirected_edge(G, 1, 2);

    // Vertex 1 is the root, neighbors {0,2} both qualify by reorder and core.
    std::vector<int32_t> core    = {3, 3, 3};
    std::vector<int32_t> reorder = {0, 1, 2};

    int32_t best_clique_size = 3;  // Larger than the number of qualified neighbors (2).
    double phi = 0.5;

    NeighborSearch(G, /*v=*/1, core, reorder, best_clique_size, phi);

    // Filter 1 should prune, best_clique_size stays unchanged.
    EXPECT_EQ(best_clique_size, 3);
}


// ---------------------------------------------------------------------------
// Filter 2 tests (Algorithm 4)
// ---------------------------------------------------------------------------
//
// Filter 2 keeps only vertices u in N for which
//   intersect_size_gt_bool(G[u], N_set, best_clique_size - 2) == true
// i.e., |N(u) ∩ N| > best_clique_size - 2.
//

// Case: Only one neighbor has enough intersection with N, others are removed.
TEST(NeighborSearchFilter2, RemovesVerticesWithSmallIntersection) {
    // Vertices: 0 is root, N = {1,2,3}
    // Edges:
    // 0 connected to 1,2,3
    // 1 connected to 2 and 3   -> intersection with N is {2,3} (size 2)
    // 2 connected only to 1    -> intersection with N is {1}   (size 1)
    // 3 connected only to 1    -> intersection with N is {1}   (size 1)
    //
    // With best_clique_size = 3, threshold = 1, so only vertex 1 stays after Filter 2.

    AdjList G(4);
    add_undirected_edge(G, 0, 1);
    add_undirected_edge(G, 0, 2);
    add_undirected_edge(G, 0, 3);

    add_undirected_edge(G, 1, 2);
    add_undirected_edge(G, 1, 3);
    // No edge between 2 and 3

    std::vector<int32_t> core    = {3, 3, 3, 3};
    std::vector<int32_t> reorder = {0, 1, 2, 3};

    int32_t best_clique_size = 3;
    double phi = 0.5;

    NeighborSearch(G, /*v=*/0, core, reorder, best_clique_size, phi);

    // Because after Filter 2 only one vertex remains in N (size 1),
    // and |N| < best_clique_size, the function returns early.
    // best_clique_size should stay unchanged.
    EXPECT_EQ(best_clique_size, 3);
}

// Case: All neighbors are highly interconnected, so Filter 2 keeps all of them.
TEST(NeighborSearchFilter2, KeepsVerticesWithLargeIntersection) {
    // Complete graph K4 on vertices {0,1,2,3}.
    // For v = 0, N = {1,2,3}.
    // Each neighbor sees 2 others inside N, so for best_clique_size = 3:
    // threshold = 1, |N(u) ∩ N| = 2 > 1, all are kept.

    AdjList G(4);
    add_undirected_edge(G, 0, 1);
    add_undirected_edge(G, 0, 2);
    add_undirected_edge(G, 0, 3);
    add_undirected_edge(G, 1, 2);
    add_undirected_edge(G, 1, 3);
    add_undirected_edge(G, 2, 3);

    std::vector<int32_t> core    = {3, 3, 3, 3};
    std::vector<int32_t> reorder = {0, 1, 2, 3};

    int32_t best_clique_size = 2;  // Start from a smaller known clique.
    double phi = 0.9;              // High enough so density <= phi -> fallback MC.

    NeighborSearch(G, /*v=*/0, core, reorder, best_clique_size, phi);

    // The maximum clique containing vertex 0 in K4 has size 4.
    EXPECT_EQ(best_clique_size, 4);
}


// ---------------------------------------------------------------------------
// Filter 3 + density decision tests (Algorithm 3 + k-VC / MC branch)
// ---------------------------------------------------------------------------
//
// We test the two main branches:
//  - dense case: density > phi -> use k-Vertex-Cover on complement
//  - sparse case: density <= phi -> use maximum_clique_induced (Bron–Kerbosch)
// Both tests use K4 so the true maximum clique size is 4.
//

// Dense case: density > phi, so the k-Vertex-Cover branch is used.
TEST(NeighborSearchDensity, DenseCaseUsesKVertexCoverBranchAndFindsClique) {
    // Same K4 graph as before.
    AdjList G(4);
    add_undirected_edge(G, 0, 1);
    add_undirected_edge(G, 0, 2);
    add_undirected_edge(G, 0, 3);
    add_undirected_edge(G, 1, 2);
    add_undirected_edge(G, 1, 3);
    add_undirected_edge(G, 2, 3);

    std::vector<int32_t> core    = {3, 3, 3, 3};
    std::vector<int32_t> reorder = {0, 1, 2, 3};

    int32_t best_clique_size = 2;
    // N = {1,2,3}, edges among N = 3, density = 3 / (3 * 2) = 0.5.
    // Choose phi < 0.5 so that density > phi and we go into k-VC branch.
    double phi = 0.1;

    NeighborSearch(G, /*v=*/0, core, reorder, best_clique_size, phi);

    EXPECT_EQ(best_clique_size, 4);
}

// Sparse case: density <= phi, so the Bron–Kerbosch-based fallback is used.
TEST(NeighborSearchDensity, SparseCaseUsesMaximumCliqueFallback) {
    // Same K4 graph.
    AdjList G(4);
    add_undirected_edge(G, 0, 1);
    add_undirected_edge(G, 0, 2);
    add_undirected_edge(G, 0, 3);
    add_undirected_edge(G, 1, 2);
    add_undirected_edge(G, 1, 3);
    add_undirected_edge(G, 2, 3);

    std::vector<int32_t> core    = {3, 3, 3, 3};
    std::vector<int32_t> reorder = {0, 1, 2, 3};

    int32_t best_clique_size = 2;
    // Now choose phi > 0.5 so that density <= phi and we take the fallback.
    double phi = 0.9;

    NeighborSearch(G, /*v=*/0, core, reorder, best_clique_size, phi);

    EXPECT_EQ(best_clique_size, 4);
}

// ---------------------------------------------------------------------------
// Algorithm-selection density boundary tests
// ---------------------------------------------------------------------------
//
// Here we focus on the last part of NeighborSearch:
//   - computing density
//   - comparing it to phi
//   - choosing between k-VC branch and MC fallback
//
// We use a small triangle graph (K3) where the true maximum clique size is 3.
//

// Helper: build a triangle K3 on vertices {0,1,2}.
static AdjList make_triangle_K3() {
    AdjList G(3);
    add_undirected_edge(G, 0, 1);
    add_undirected_edge(G, 0, 2);
    add_undirected_edge(G, 1, 2);
    return G;
}

// Case 1: phi < density  -> the code should take the "dense" branch,
// but from the outside we only verify that it still finds the maximum clique.
TEST(NeighborSearchAlgorithmChoice, PhiBelowDensityStillFindsMaxClique) {
    AdjList G = make_triangle_K3();

    // All vertices have the same core value, and reorder is increasing.
    std::vector<int32_t> core    = {3, 3, 3};
    std::vector<int32_t> reorder = {0, 1, 2};

    // Start with a known clique of size 2 (an edge).
    int32_t best_clique_size = 2;

    // For the induced neighborhood N = {1,2}, the internal "edge count"
    // gives density = 1.0. Here phi < density, so the dense branch is chosen.
    double phi = 0.3;

    NeighborSearch(G, /*v=*/0, core, reorder, best_clique_size, phi);

    // The maximum clique containing vertex 0 in K3 has size 3.
    EXPECT_EQ(best_clique_size, 3);
}

// Case 2: phi == density  -> density > phi is false,
// so we expect the code to take the fallback MC branch.
// Again, from the outside we only check correctness of the result.
TEST(NeighborSearchAlgorithmChoice, PhiEqualToDensityUsesFallbackAndFindsMaxClique) {
    AdjList G = make_triangle_K3();

    std::vector<int32_t> core    = {3, 3, 3};
    std::vector<int32_t> reorder = {0, 1, 2};

    int32_t best_clique_size = 2;

    // For N = {1,2}, density = 1.0. Here phi == density, so
    // (density > phi) is false and the MC fallback should be used.
    double phi = 1.0;

    NeighborSearch(G, /*v=*/0, core, reorder, best_clique_size, phi);

    EXPECT_EQ(best_clique_size, 3);
}

// Case 3: phi > density  -> also forces the fallback branch.
// This is a more "extreme" version of the previous test.
TEST(NeighborSearchAlgorithmChoice, PhiAboveDensityUsesFallbackAndFindsMaxClique) {
    AdjList G = make_triangle_K3();

    std::vector<int32_t> core    = {3, 3, 3};
    std::vector<int32_t> reorder = {0, 1, 2};

    int32_t best_clique_size = 2;

    // density = 1.0, choose a phi well above that.
    double phi = 5.0;

    NeighborSearch(G, /*v=*/0, core, reorder, best_clique_size, phi);

    EXPECT_EQ(best_clique_size, 3);
}


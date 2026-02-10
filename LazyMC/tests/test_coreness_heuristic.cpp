#include <gtest/gtest.h>

#include "lazymc/kcore.h"
#include "lazymc/determine_sort_order.h"
#include "lazymc/lazygraph.h"
#include "lazymc/coreness_heuristic.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <unordered_set>
#include <vector>

using lazymc::AdjList;
using lazymc::KCore;
using lazymc::DetermineSortOrder;
using lazymc::LazyGraph;
using lazymc::coreness_based_heuristic;

static bool IsCliqueInG(const AdjList& G, const std::vector<int32_t>& C) {
    const std::size_t n = G.size();

    std::vector<std::unordered_set<int32_t>> adj(n);
    for (std::size_t v = 0; v < n; ++v) {
        for (int32_t u : G[v]) adj[v].insert(u);
    }

    for (std::size_t i = 0; i < C.size(); ++i) {
        int32_t a = C[i];
        if (a < 0 || static_cast<std::size_t>(a) >= n) return false;
        for (std::size_t j = i + 1; j < C.size(); ++j) {
            int32_t b = C[j];
            if (b < 0 || static_cast<std::size_t>(b) >= n) return false;
            if (adj[static_cast<std::size_t>(a)].count(b) == 0) return false;
        }
    }
    return true;
}

struct BuiltH {
    LazyGraph H;
    std::vector<int32_t> core_H;

    BuiltH(const AdjList& G, std::vector<int32_t> coreH,
           const std::vector<int32_t>& r2o, const std::vector<int32_t>& o2r)
        : H(G, r2o, o2r, coreH), core_H(std::move(coreH)) {}
};

static BuiltH BuildHFromAlgo1Pieces(const AdjList& G, int32_t clique_lb = 0) {
    std::vector<int32_t> core_G = KCore(G, clique_lb);
    auto order = DetermineSortOrder(G, core_G);

    std::vector<int32_t> core_H(order.relabel_to_original.size(), 0);
    for (std::size_t vH = 0; vH < order.relabel_to_original.size(); ++vH) {
        const int32_t vG = order.relabel_to_original[vH];
        core_H[vH] = core_G[static_cast<std::size_t>(vG)];
    }

    return BuiltH(G, core_H, order.relabel_to_original, order.original_to_relabel);
}

TEST(CorenessHeuristic, EmptyGraphReturnsEmpty) {
    AdjList G;
    auto built = BuildHFromAlgo1Pieces(G);
    auto C = coreness_based_heuristic(built.H, built.core_H);
    EXPECT_TRUE(C.empty());
}

TEST(CorenessHeuristic, SingleVertexGraph) {
    AdjList G = { {} };
    auto built = BuildHFromAlgo1Pieces(G);
    auto C = coreness_based_heuristic(built.H, built.core_H);

    // Either {} or {0} is acceptable; if non-empty it must be a clique.
    EXPECT_TRUE(IsCliqueInG(G, C));
    EXPECT_LE(C.size(), 1u);
}

TEST(CorenessHeuristic, NoEdgesReturnsAtMostOneVertex) {
    AdjList G = { {}, {}, {}, {} };
    auto built = BuildHFromAlgo1Pieces(G);
    auto C = coreness_based_heuristic(built.H, built.core_H);

    EXPECT_TRUE(IsCliqueInG(G, C));
    EXPECT_LE(C.size(), 1u);
}

TEST(CorenessHeuristic, TriangleFindsCliqueOfSize3) {
    AdjList G = {
        {1, 2},
        {0, 2},
        {0, 1}
    };
    auto built = BuildHFromAlgo1Pieces(G);
    auto C = coreness_based_heuristic(built.H, built.core_H);

    EXPECT_TRUE(IsCliqueInG(G, C));
    EXPECT_EQ(C.size(), 3u);
}

TEST(CorenessHeuristic, K4FindsCliqueOfSize4) {
    AdjList G = {
        {1,2,3},
        {0,2,3},
        {0,1,3},
        {0,1,2}
    };
    auto built = BuildHFromAlgo1Pieces(G);
    auto C = coreness_based_heuristic(built.H, built.core_H);

    EXPECT_TRUE(IsCliqueInG(G, C));
    EXPECT_EQ(C.size(), 4u);
}

TEST(CorenessHeuristic, DisconnectedComponentsChoosesLargestClique) {
    // Component A: triangle (0,1,2)
    // Component B: edge (3,4)
    // Component C: isolated (5)
    AdjList G = {
        {1,2},    // 0
        {0,2},    // 1
        {0,1},    // 2
        {4},      // 3
        {3},      // 4
        {}        // 5
    };

    auto built = BuildHFromAlgo1Pieces(G);
    auto C = coreness_based_heuristic(built.H, built.core_H);

    EXPECT_TRUE(IsCliqueInG(G, C));
    EXPECT_EQ(C.size(), 3u);
}

TEST(CorenessHeuristic, CliqueWithTailStillFindsClique) {
    // K4 on {0,1,2,3}, plus a tail 3-4-5 path
    AdjList G = {
        {1,2,3},      // 0
        {0,2,3},      // 1
        {0,1,3},      // 2
        {0,1,2,4},    // 3
        {3,5},        // 4
        {4}           // 5
    };

    auto built = BuildHFromAlgo1Pieces(G);
    auto C = coreness_based_heuristic(built.H, built.core_H);

    EXPECT_TRUE(IsCliqueInG(G, C));
    EXPECT_GE(C.size(), 4u); // Should at least find the K4
}

TEST(CorenessHeuristic, DeterministicAcrossCalls) {
    AdjList G = {
        {1,2,3},
        {0,2},
        {0,1,3},
        {0,2},
        {} // isolated
    };

    auto built = BuildHFromAlgo1Pieces(G);
    auto A = coreness_based_heuristic(built.H, built.core_H);
    auto B = coreness_based_heuristic(built.H, built.core_H);

    EXPECT_TRUE(IsCliqueInG(G, A));
    EXPECT_TRUE(IsCliqueInG(G, B));

    std::sort(A.begin(), A.end());
    std::sort(B.begin(), B.end());
    EXPECT_EQ(A, B);
}


#include <random>

TEST(CorenessHeuristic, RandomGraphsWithFixedSeedReturnClique) {
    constexpr std::uint32_t seed = 1337;
    std::mt19937 rng(seed);

    const int num_tests = 20;
    const int n = 12;               // number of vertices
    const double p = 0.35;          // edge probability

    std::bernoulli_distribution edge(p);

    for (int t = 0; t < num_tests; ++t) {
        AdjList G(n);

        // Generate random undirected graph
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (edge(rng)) {
                    G[i].push_back(j);
                    G[j].push_back(i);
                }
            }
        }

        // Build H exactly as in Algorithm 1
        auto built = BuildHFromAlgo1Pieces(G);

        // Run coreness-based heuristic
        auto C = coreness_based_heuristic(built.H, built.core_H);

        // Must always return a valid clique (possibly empty)
        EXPECT_TRUE(IsCliqueInG(G, C))
            << "Failed on random test " << t
            << " with seed " << seed;
    }
}

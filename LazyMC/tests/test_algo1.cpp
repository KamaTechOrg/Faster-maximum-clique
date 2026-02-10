#include <gtest/gtest.h>

#include "lazymc/kcore.h"
#include "lazymc/determine_sort_order.h"
#include "lazymc/lazygraph.h"
#include "lazymc/lazymc.h"

#include <unordered_set>
#include <cstddef>
#include <cstdint>
#include <vector>

using lazymc::AdjList;
using lazymc::KCore;
using lazymc::DetermineSortOrder;
using lazymc::LazyGraph;

TEST(Algo1Wiring, KCoreToOrderToLazyGraphMappingIsConsistent) {
    // Star graph centered at 0:
    // 0 connected to 1..4, leaves only connect to 0.
    AdjList G = {
        {1,2,3,4}, // 0
        {0},       // 1
        {0},       // 2
        {0},       // 3
        {0}        // 4
    };

    // Step 4: compute core
    std::vector<int32_t> core = KCore(G, /*min_degree=*/0);
    ASSERT_EQ(core.size(), G.size());

    // Step 5: determine order
    auto order = DetermineSortOrder(G, core);
    ASSERT_EQ(order.relabel_to_original.size(), G.size());
    ASSERT_EQ(order.original_to_relabel.size(), G.size());

    // Order must be a permutation and inverse must be consistent
    for (std::size_t vH = 0; vH < G.size(); ++vH) {
        int32_t vG = order.relabel_to_original[vH];
        ASSERT_GE(vG, 0);
        ASSERT_LT(static_cast<std::size_t>(vG), G.size());
        EXPECT_EQ(order.original_to_relabel[static_cast<std::size_t>(vG)],
                  static_cast<int32_t>(vH));
    }

    // Step 6: build LazyGraph
    LazyGraph H(G, order.relabel_to_original, order.original_to_relabel, core);

    // Consistency check:
    // For every vH, map to vG, take neighbors in original G,
    // map them to H-labels, and verify they appear in H.hashed_neighbors(vH).
    for (int32_t vH = 0; vH < static_cast<int32_t>(G.size()); ++vH) {
        const int32_t vG = H.to_original(vH);
        auto& hv = H.hashed_neighbors(vH);

        for (int32_t uG : G[static_cast<std::size_t>(vG)]) {
            const int32_t uH = H.to_relabelled(uG);
            EXPECT_EQ(hv.count(uH), 1u);
        }

        // Also check the size matches degree (full neighbors, no filtering yet)
        EXPECT_EQ(hv.size(), G[static_cast<std::size_t>(vG)].size());
    }
}

TEST(Algo1Wiring, OrderIsNonIncreasingByCore) {
    // A small graph where core values are likely not all equal.
    AdjList G = {
        {1,2,3}, // 0
        {0,2},   // 1
        {0,1,3}, // 2
        {0,2},   // 3
        {}       // 4 isolated
    };

    std::vector<int32_t> core = KCore(G, /*min_degree=*/0);
    auto order = DetermineSortOrder(G, core);

    // core along relabel_to_original must be non-increasing
    for (std::size_t i = 1; i < order.relabel_to_original.size(); ++i) {
        int32_t prevG = order.relabel_to_original[i - 1];
        int32_t currG = order.relabel_to_original[i];
        EXPECT_GE(core[static_cast<std::size_t>(prevG)],
                  core[static_cast<std::size_t>(currG)]);
    }
}




static bool IsClique(const AdjList& G, const std::vector<int32_t>& C) {
    const std::size_t n = G.size();
    std::vector<std::unordered_set<int32_t>> adj(n);
    for (std::size_t v = 0; v < n; ++v) {
        for (int32_t u : G[v]) adj[v].insert(u);
    }
    for (std::size_t i = 0; i < C.size(); ++i) {
        for (std::size_t j = i + 1; j < C.size(); ++j) {
            int32_t a = C[i], b = C[j];
            if (a < 0 || b < 0) return false;
            if (static_cast<std::size_t>(a) >= n || static_cast<std::size_t>(b) >= n) return false;
            if (adj[static_cast<std::size_t>(a)].count(b) == 0) return false;
        }
    }
    return true;
}

TEST(Algo1Wiring, LazyMCRunReturnsCliqueOnTriangle) {
    AdjList G = {
        {1,2},
        {0,2},
        {0,1}
    };

    auto C = lazymc::LazyMC_run(G);
    EXPECT_TRUE(IsClique(G, C));
    EXPECT_GE(C.size(), 2u);
}

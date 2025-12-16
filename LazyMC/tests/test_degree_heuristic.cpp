#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "lazymc/degree_heuristic.h"
#include "lazymc/lazygraph.h"

namespace {

// Helper to build a LazyGraph with identity relabeling (H-ids == G-ids).
lazymc::LazyGraph make_identity_lazygraph(const lazymc::AdjList& G) {
    const int32_t n = static_cast<int32_t>(G.size());

    std::vector<int32_t> relabel_to_original(n);
    std::vector<int32_t> original_to_relabel(n);
    std::vector<int32_t> core(n, 0);

    for (int32_t i = 0; i < n; ++i) {
        relabel_to_original[i] = i;
        original_to_relabel[i] = i;
    }

    return lazymc::LazyGraph(G, relabel_to_original, original_to_relabel, core);
}

} // namespace

TEST(DegreeBasedHeuristicTest, EmptyGraph) {
    lazymc::AdjList G;
    auto H = make_identity_lazygraph(G);
    auto result = lazymc::degree_based_heuristic(H, 5);
    EXPECT_TRUE(result.empty());
}

TEST(DegreeBasedHeuristicTest, SingleVertex) {
    lazymc::AdjList G = {{}};
    auto H = make_identity_lazygraph(G);
    auto result = lazymc::degree_based_heuristic(H, 5);
    EXPECT_EQ(result.size(), 1);
}

TEST(DegreeBasedHeuristicTest, Triangle) {
    lazymc::AdjList G = {
        {1, 2},
        {0, 2},
        {0, 1}
    };
    auto H = make_identity_lazygraph(G);
    auto result = lazymc::degree_based_heuristic(H, 3);
    EXPECT_EQ(result.size(), 3);
}

TEST(DegreeBasedHeuristicTest, K4Clique) {
    lazymc::AdjList G = {
        {1, 2, 3},
        {0, 2, 3},
        {0, 1, 3},
        {0, 1, 2}
    };
    auto H = make_identity_lazygraph(G);
    auto result = lazymc::degree_based_heuristic(H, 4);
    EXPECT_EQ(result.size(), 4);
}

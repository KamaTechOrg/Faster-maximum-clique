// Cross-checks the bitset variants against the original implementations.
#include "lazymc/bk.h"
#include "lazymc/bk_bitset.h"
#include "lazymc/incumbent.h"
#include "lazymc/k_vertex_cover.h"
#include "lazymc/k_vertex_cover_bitset.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

namespace {

lazymc::AdjList random_graph(int n, double p, std::mt19937& rng) {
    std::bernoulli_distribution e(p);
    lazymc::AdjList G(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (e(rng)) { G[i].push_back(j); G[j].push_back(i); }
    return G;
}

// Complement of G[verts], indices = positions in verts (what the original path builds).
lazymc::AdjList induced_complement(const lazymc::AdjList& G, const std::vector<int32_t>& verts) {
    const std::size_t m = verts.size();
    lazymc::AdjList C(m);
    for (std::size_t i = 0; i < m; ++i)
        for (std::size_t j = i + 1; j < m; ++j) {
            const auto& nb = G[static_cast<std::size_t>(verts[i])];
            if (std::find(nb.begin(), nb.end(), verts[j]) == nb.end()) {
                C[i].push_back(static_cast<int32_t>(j));
                C[j].push_back(static_cast<int32_t>(i));
            }
        }
    return C;
}

bool is_cover(const lazymc::AdjList& C, const std::vector<int32_t>& cover) {
    std::vector<char> in(C.size(), 0);
    for (int32_t v : cover) in[static_cast<std::size_t>(v)] = 1;
    for (std::size_t u = 0; u < C.size(); ++u)
        for (int32_t v : C[u])
            if (!in[u] && !in[static_cast<std::size_t>(v)]) return false;
    return true;
}

bool is_clique(const lazymc::AdjList& G, const std::vector<int32_t>& c) {
    for (std::size_t i = 0; i < c.size(); ++i)
        for (std::size_t j = i + 1; j < c.size(); ++j) {
            const auto& nb = G[static_cast<std::size_t>(c[i])];
            if (std::find(nb.begin(), nb.end(), c[j]) == nb.end()) return false;
        }
    return true;
}

} // namespace

TEST(BitsetVariants, KVertexCoverMatchesOriginal) {
    std::mt19937 rng(12345);
    for (int t = 0; t < 300; ++t) {
        const int n = 5 + static_cast<int>(rng() % 120);    // crosses 64-bit word boundaries
        const double p = 0.6 + (rng() % 35) / 100.0;        // dense G -> sparse complement
        auto G = random_graph(n + 10, p, rng);

        std::vector<int32_t> verts(static_cast<std::size_t>(n + 10));
        std::iota(verts.begin(), verts.end(), 0);
        std::shuffle(verts.begin(), verts.end(), rng);
        verts.resize(static_cast<std::size_t>(n));           // arbitrary order, like N_G

        const auto C = induced_complement(G, verts);
        for (int k : {0, n / 8, n / 4, n / 3, n / 2, n}) {
            const auto a = lazymc::k_vertex_cover_adj(C, k);
            const auto b = lazymc::k_vertex_cover_induced_complement_bitset(G, verts, k);
            ASSERT_EQ(a.aborted, b.aborted) << "t=" << t << " k=" << k;
            ASSERT_EQ(a.found, b.found) << "t=" << t << " k=" << k;
            if (b.found) {
                EXPECT_LE(static_cast<int>(b.cover.size()), k);
                EXPECT_TRUE(is_cover(C, b.cover));
                auto ac = a.cover, bc = b.cover;
                std::sort(ac.begin(), ac.end());
                EXPECT_EQ(ac, bc) << "same search order should give the same cover";
            }
        }
    }
}

TEST(BitsetVariants, BronKerboschMaximumMatchesOriginalSize) {
    std::mt19937 rng(777);
    for (int t = 0; t < 150; ++t) {
        const int n = 5 + static_cast<int>(rng() % 70);  // original BK is slow on big dense graphs
        const double p = 0.1 + (rng() % 70) / 100.0;
        auto G = random_graph(n, p, rng);
        for (auto& nb : G) std::sort(nb.begin(), nb.end());

        lazymc::AdjMap M;
        std::vector<int32_t> verts(static_cast<std::size_t>(n));
        std::iota(verts.begin(), verts.end(), 0);
        for (int v = 0; v < n; ++v) M[v] = G[static_cast<std::size_t>(v)];

        lazymc::reset_incumbent();
        const auto a = lazymc::bron_kerbosch_maximum(M, 0);
        lazymc::reset_incumbent();
        const auto b = lazymc::bron_kerbosch_maximum_bitset(G, verts, 0);
        lazymc::reset_incumbent();

        ASSERT_EQ(a.size(), b.size()) << "t=" << t;
        EXPECT_TRUE(is_clique(G, b));
    }
}

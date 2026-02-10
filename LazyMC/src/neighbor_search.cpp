#include "lazymc/neighbor_search.h"

#include "lazymc/k_vertex_cover.h"
#include "lazymc/intersect_functions.h"
#include "lazymc/bk.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <unordered_set>
#include <vector>

using lazymc::AdjMap;
using lazymc::bron_kerbosch_maximum;

namespace {

using lazymc::AdjList;
using lazymc::k_vertex_cover_adj;
using lazymc::k_vertex_cover_res;

// Run k-Vertex-Cover on the complement of the induced subgraph G[verts].
k_vertex_cover_res k_vertex_cover_induced_complement(
    const AdjList& G,
    const std::vector<int32_t>& verts,
    int32_t k
) {
    const std::size_t m = verts.size();
    AdjList subG(m);

    std::vector<int32_t> old_to_new(G.size(), -1);
    for (std::size_t i = 0; i < m; ++i) {
        int32_t v = verts[i];
        old_to_new[static_cast<std::size_t>(v)] = static_cast<int32_t>(i);
    }

    std::vector<std::uint8_t> is_neighbor(G.size(), 0);

    for (std::size_t i = 0; i < m; ++i) {
        int32_t v = verts[i];

        for (int32_t u : G[static_cast<std::size_t>(v)]) {
            if (u >= 0 && static_cast<std::size_t>(u) < G.size() &&
                old_to_new[static_cast<std::size_t>(u)] != -1) {
                is_neighbor[static_cast<std::size_t>(u)] = 1;
            }
        }

        for (std::size_t j = i + 1; j < m; ++j) {
            int32_t u = verts[j];
            if (!is_neighbor[static_cast<std::size_t>(u)]) {
                subG[i].push_back(static_cast<int32_t>(j));
                subG[j].push_back(static_cast<int32_t>(i));
            }
        }

        for (int32_t u : G[static_cast<std::size_t>(v)]) {
            if (u >= 0 && static_cast<std::size_t>(u) < G.size() &&
                old_to_new[static_cast<std::size_t>(u)] != -1) {
                is_neighbor[static_cast<std::size_t>(u)] = 0;
            }
        }
    }

    return k_vertex_cover_adj(subG, k);
}

// Build induced subgraph for {v} ∪ N and run MC search; return the clique (G-labeling).
std::vector<int32_t> maximum_clique_induced(
    const AdjList& G,
    const std::vector<int32_t>& N,
    int32_t v
) {
    const std::size_t n_vertices = G.size();
    if (v < 0 || static_cast<std::size_t>(v) >= n_vertices) return {};

    std::vector<int32_t> subset;
    subset.reserve(N.size() + 1);
    subset.push_back(v);

    std::vector<std::uint8_t> in_subset(n_vertices, 0);
    in_subset[static_cast<std::size_t>(v)] = 1;

    for (int32_t u : N) {
        if (u >= 0 && static_cast<std::size_t>(u) < n_vertices &&
            !in_subset[static_cast<std::size_t>(u)]) {
            in_subset[static_cast<std::size_t>(u)] = 1;
            subset.push_back(u);
        }
    }

    AdjMap subG;

    for (int32_t x : subset) {
        const auto& nbrs = G[static_cast<std::size_t>(x)];
        std::vector<int32_t> subNbrs;
        subNbrs.reserve(nbrs.size());

        for (int32_t y : nbrs) {
            if (y >= 0 && static_cast<std::size_t>(y) < n_vertices &&
                in_subset[static_cast<std::size_t>(y)]) {
                subNbrs.push_back(y);
            }
        }

        std::sort(subNbrs.begin(), subNbrs.end());
        subNbrs.erase(std::unique(subNbrs.begin(), subNbrs.end()), subNbrs.end());
        subG[x] = std::move(subNbrs);
    }

    return bron_kerbosch_maximum(subG); // returns vertices in G-labeling
}

} // anonymous namespace

namespace lazymc {

void NeighborSearch(LazyGraph& H,
                    int32_t vH,
                    const std::vector<int32_t>& core_H,
                    std::vector<int32_t>& best_clique_G,
                    double phi) {
    const int32_t best = static_cast<int32_t>(best_clique_G.size());
    const int32_t nH = static_cast<int32_t>(H.num_vertices());
    if (vH < 0 || vH >= nH) return;

    // -----------------------
    // Filter 1
    // N ← {u ∈ N^+(v) : c[u] ≥ |C*|}
    // -----------------------
    std::vector<int32_t> N; // H-labeling
    const auto& Nv = H.sorted_neighbors(vH);
    N.reserve(Nv.size());

    for (int32_t uH : Nv) {
        if (uH > vH && core_H[static_cast<std::size_t>(uH)] >= best) {
            N.push_back(uH);
        }
    }
    if (static_cast<int32_t>(N.size()) < best) return;

    // -----------------------
    // Filter 2 (Algorithm 4)
    // keep u if |N(u) ∩ N| > best - 2
    // -----------------------
    std::unordered_set<int32_t> N_set(N.begin(), N.end());
    std::vector<int32_t> tempN;
    tempN.reserve(N.size());

    for (int32_t uH : N) {
        const auto& Nu = H.sorted_neighbors(uH); // vector<int32_t> in H-labeling
        if (intersect_size_gt_bool(Nu, N_set, best - 2)) {
            tempN.push_back(uH);
        }
    }

    N = std::move(tempN);
    if (static_cast<int32_t>(N.size()) < best) return;

    // -----------------------
    // Filter 3 (Algorithm 3) -- PASS 1: keep only strong vertices
    // -----------------------
    N_set.clear();
    N_set.insert(N.begin(), N.end());

    std::vector<int32_t> tempN2;
    tempN2.reserve(N.size());
    std::vector<int32_t> tmp_intersection;

    for (int32_t uH : N) {
        const auto& Nu = H.sorted_neighbors(uH);
        const int32_t d = intersect_gt(Nu, N_set, tmp_intersection, best - 2);
        if (d > best - 2) {
            tempN2.push_back(uH);
        }
    }

    N = std::move(tempN2);
    if (static_cast<int32_t>(N.size()) < best) return;

    // -----------------------
    // PASS 2: recompute density based ONLY on final N
    // -----------------------
    N_set.clear();
    N_set.insert(N.begin(), N.end());

    int64_t count_edge = 0;
    tmp_intersection.clear();

    for (int32_t uH : N) {
        const auto& Nu = H.sorted_neighbors(uH);
        const int32_t d = intersect_gt(Nu, N_set, tmp_intersection, 0); // full intersection size
        if (d > 0) count_edge += d;
    }

    const double denom = static_cast<double>(N.size()) * static_cast<double>(N.size() - 1);
    const double density = (denom > 0.0) ? (static_cast<double>(count_edge) / denom) : 0.0;

    // Convert N from H-labeling to G-labeling for kVC/BK code
    std::vector<int32_t> N_G;
    N_G.reserve(N.size());
    for (int32_t uH : N) {
        N_G.push_back(H.to_original(uH));
    }

    const int32_t vG = H.to_original(vH);
    const AdjList& G = H.original_graph();

    // -----------------------
    // Dense → k-VertexCover on complement, else → MaximumCliqueSearch
    // -----------------------
    if (density > phi) {
        const int32_t k = static_cast<int32_t>(N_G.size()) - best;

        k_vertex_cover_res res = k_vertex_cover_induced_complement(G, N_G, k);

        // Case 1: aborted → fallback to BK
        if (res.aborted) {
            std::vector<int32_t> clique = maximum_clique_induced(G, N_G, vG);
            if (static_cast<int32_t>(clique.size()) > best) {
                best_clique_G = std::move(clique);
            }
            return;
        }

        // Case 2: finished and NO vertex cover → safe to conclude NO larger clique
        if (!res.found) {
            return;
        }

        // Case 3: found vertex cover → clique is complement of cover inside N plus v
        const int32_t m = static_cast<int32_t>(N_G.size());
        std::vector<std::uint8_t> in_cover(static_cast<std::size_t>(m), 0);

        for (int32_t idx : res.cover) {
            if (idx >= 0 && idx < m) in_cover[static_cast<std::size_t>(idx)] = 1;
        }

        std::vector<int32_t> clique;
        clique.reserve(static_cast<std::size_t>(m) + 1);
        clique.push_back(vG);
        for (int32_t i = 0; i < m; ++i) {
            if (!in_cover[static_cast<std::size_t>(i)]) {
                clique.push_back(N_G[static_cast<std::size_t>(i)]);
            }
        }

        if (static_cast<int32_t>(clique.size()) > best) {
            best_clique_G = std::move(clique);
        }
        return;
    }

    // Not dense: BK fallback
    {
        std::vector<int32_t> clique = maximum_clique_induced(G, N_G, vG);
        if (static_cast<int32_t>(clique.size()) > best) {
            best_clique_G = std::move(clique);
        }
    }
}

} // namespace lazymc

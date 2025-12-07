#include "lazymc/k_vertex_cover.h"
#include <cstdint>
#include <vector>
#include <unordered_set>
#include "lazymc/intersect_functions.h"
#include <algorithm>   
#include "lazymc/bk.h"



using lazymc::AdjMap;
using lazymc::bron_kerbosch_maximum;


namespace {

using lazymc::AdjList;
using lazymc::k_vertex_cover_res;
using lazymc::k_vertex_cover_adj;

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

// Build induced subgraph for {v} ∪ N and run MC search
void maximum_clique_induced(
    const AdjList& G,
    const std::vector<int32_t>& N,
    int32_t v,
    int32_t& best_clique_size
) {
    const std::size_t n_vertices = G.size();
    if (v < 0 || static_cast<std::size_t>(v) >= n_vertices) return;

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

    std::vector<int32_t> clique = bron_kerbosch_maximum(subG);
    int32_t clique_size = static_cast<int32_t>(clique.size());
    if (clique_size > best_clique_size) {
        best_clique_size = clique_size;
    }
}

} // anonymous namespace


namespace lazymc {

void NeighborSearch(const std::vector<std::vector<int32_t>>& G,
                    int32_t v,
                    const std::vector<int32_t>& core,
                    const std::vector<int32_t>& reorder,
                    int32_t& best_clique_size,
                    double phi)
{
    // -----------------------
    // Filter 1
    // -----------------------
    std::vector<int32_t> N;
    for (int32_t u : G[v]) {
        if (reorder[u] > reorder[v] && core[u] >= best_clique_size) {
            N.push_back(u);
        }
    }
    if (N.size() < best_clique_size) return;

    // -----------------------
    // Filter 2 (Algorithm 4)
    // -----------------------
    std::unordered_set<int32_t> N_set(N.begin(), N.end());
    std::vector<int32_t> tempN;

    for (int32_t u : N) {
        if (intersect_size_gt_bool(G[u], N_set, best_clique_size - 2)) {
            tempN.push_back(u);
        }
    }

    N = std::move(tempN);
    if (N.size() < best_clique_size) return;

    // -----------------------
    // Filter 3 (Algorithm 3)
    // -----------------------
    N_set.clear();
    N_set.insert(N.begin(), N.end());

    int64_t count_edge = 0;
    std::vector<int32_t> tempN2;
    std::vector<int32_t> C_temp;

    for (int32_t u : N) {
        int32_t d = intersect_gt(G[u], N_set, C_temp, best_clique_size - 2);

        if (d > best_clique_size - 2) {
            tempN2.push_back(u);
        }

        if (d > 0) count_edge += d;
    }

    N = std::move(tempN2);
    if (N.size() < best_clique_size) return;

    // -----------------------
    // Density decision
    // -----------------------
    double density = static_cast<double>(count_edge) /
                     static_cast<double>(N.size() * (N.size() - 1));

    if (density > phi) {
        int32_t k = static_cast<int32_t>(N.size()) - best_clique_size;

        k_vertex_cover_res res = k_vertex_cover_induced_complement(G, N, k);
        if (res.aborted || !res.found) return;

        int32_t m = static_cast<int32_t>(N.size());
        std::vector<std::uint8_t> in_cover(m, 0);

        for (int32_t idx : res.cover) {
            if (idx >= 0 && idx < m) {
                in_cover[static_cast<std::size_t>(idx)] = 1;
            }
        }

        int32_t clique_part = 0;
        for (int i = 0; i < m; ++i) {
            if (!in_cover[i]) clique_part++;
        }

        int32_t new_clique_size = clique_part + 1;
        if (new_clique_size > best_clique_size) {
            best_clique_size = new_clique_size;
        }

        return;
    }

    // fallback: run MC on induced graph
    maximum_clique_induced(G, N, v, best_clique_size);
}

} // namespace lazymc

#include "lazymc/lazygraph.h"
#include "lazymc/intersect_functions.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace lazymc {

std::vector<int32_t> degree_based_heuristic(LazyGraph& H, int top_k) {
    const int32_t n = static_cast<int32_t>(H.num_vertices());
    if (n <= 0) return {};

    std::vector<int32_t> best_clique;

    // Step 1: top-K by degree (use sorted_neighbors size as degree proxy)
    std::vector<std::pair<int32_t,int32_t>> degree_vertex;
    degree_vertex.reserve(static_cast<std::size_t>(n));
    for (int32_t v = 0; v < n; ++v) {
        degree_vertex.push_back({static_cast<int32_t>(H.sorted_neighbors(v).size()), v});
    }
    std::sort(degree_vertex.begin(), degree_vertex.end(),
              [](auto& a, auto& b){ return a.first > b.first; });

    const int32_t k = std::min<int32_t>(top_k, n);

    std::vector<int32_t> tmp; // intersection buffer (reused)

    for (int32_t i = 0; i < k; ++i) {
        const int32_t v = degree_vertex[static_cast<std::size_t>(i)].second;

        // Step 3: N ← {u ∈ N(v) : deg(u) ≥ |C*|}
        std::vector<int32_t> N;
        const auto& Nv = H.sorted_neighbors(v);
        N.reserve(Nv.size());
        for (int32_t u : Nv) {
            if (static_cast<int32_t>(H.sorted_neighbors(u).size()) >=
                static_cast<int32_t>(best_clique.size())) {
                N.push_back(u);
            }
        }

        std::vector<int32_t> C;
        C.push_back(v);

        while (!N.empty()) {
            // Step 7: choose best_u by maximizing |N ∩ N(w)|
            int32_t best_u = -1;
            int32_t max_intersection = -1;

            for (int32_t w : N) {
                const auto& Bw = H.hashed_neighbors(w); // cached (built once per w)
                int32_t d = intersect_gt(N, Bw, tmp, max_intersection);
                if (d > max_intersection) {
                    max_intersection = d;
                    best_u = w;
                }
            }

            if (best_u < 0) break;

            // Step 8: C ← C ∪ {best_u}
            C.push_back(best_u);

            // Step 9: N ← N ∩ N(best_u)
            const auto& Bu = H.hashed_neighbors(best_u);
            int32_t d = intersect_gt(N, Bu, tmp, 0);

            if (d <= 0) {
                N.clear();
            } else {
                N.assign(tmp.begin(), tmp.begin() + d);
                N.erase(std::remove(N.begin(), N.end(), best_u), N.end());
            }
        }

        if (C.size() > best_clique.size()) {
            best_clique = std::move(C);
        }
    }

    return best_clique;
}

} // namespace lazymc

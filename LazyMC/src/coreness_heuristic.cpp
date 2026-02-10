#include "lazymc/coreness_heuristic.h"
#include "lazymc/intersect_functions.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace lazymc {

// Algorithm 6: Coreness-based heuristic search
std::vector<int32_t> coreness_based_heuristic(
    LazyGraph& H,
    const std::vector<int32_t>& core
) {
    const int32_t n = static_cast<int32_t>(H.num_vertices());
    if (n <= 0) return {};

    // degeneracy + 1 ≈ max core value (H-labeling)
    int32_t max_core = 0;
    for (int32_t v = 0; v < n; ++v) {
        if (core[static_cast<std::size_t>(v)] > max_core) {
            max_core = core[static_cast<std::size_t>(v)];
        }
    }

    // C* in H-labeling
    std::vector<int32_t> best_clique_H;

    // Temporary buffer for intersections
    std::vector<int32_t> tmp;

    // Algorithm 6: for k ← degeneracy + 1 downto 1
    for (int32_t k = max_core; k >= 1; --k) {

        // v ← lowest-numbered vertex with c[v] = k
        int32_t v = -1;
        for (int32_t i = 0; i < n; ++i) {
            if (core[static_cast<std::size_t>(i)] == k) {
                v = i;
                break;
            }
        }
        if (v < 0) continue;

        // N ← N_G^+(v)
        std::vector<int32_t> N;
        const auto& Nv = H.sorted_neighbors(v);
        N.reserve(Nv.size());
        for (int32_t u : Nv) {
            if (u > v) {
                N.push_back(u);
            }
        }

        // C = {v}
        std::vector<int32_t> C;
        C.push_back(v);

        // repeat
        while (!N.empty()) {

            // u ← highest-numbered vertex in N
            int32_t u = *std::max_element(N.begin(), N.end());

            // θ = |C*| − |C|
            const int32_t theta =
                static_cast<int32_t>(best_clique_H.size()) -
                static_cast<int32_t>(C.size());

            // N ← N ∩ N_G(u)  (intersect-gt)
            const auto& Bu = H.hashed_neighbors(u);
            const int32_t d = intersect_gt(N, Bu, tmp, theta);

            // C ← C ∪ {u}
            C.push_back(u);

            // until |N| == 0
            if (d <= 0) {
                N.clear();
            } else {
                N.assign(tmp.begin(), tmp.begin() + d);
                // remove u if still present
                N.erase(std::remove(N.begin(), N.end(), u), N.end());
            }
        }

        // if |C| > |C*| then C* ← C
        if (C.size() > best_clique_H.size()) {
            best_clique_H = std::move(C);
        }
    }

    // Convert clique from H-labeling to G-labeling
    std::vector<int32_t> best_clique_G;
    best_clique_G.reserve(best_clique_H.size());
    for (int32_t vH : best_clique_H) {
        best_clique_G.push_back(H.to_original(vH));
    }

    return best_clique_G;
}

} // namespace lazymc

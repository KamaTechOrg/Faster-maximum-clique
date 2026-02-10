
#include "lazymc/systematic_search.h"
#include "lazymc/neighbor_search.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace lazymc {

std::vector<int32_t> systematic_search(LazyGraph& H,
                                       const std::vector<int32_t>& core_H) {
    const int32_t n = static_cast<int32_t>(H.num_vertices());
    if (n <= 0) return {};

    // degeneracy + 1 ≈ max core value
    int32_t max_core = 0;
    for (int32_t v = 0; v < n; ++v) {
        max_core = std::max(max_core, core_H[static_cast<std::size_t>(v)]);
    }

    // Incumbent clique (in G-labeling)
    std::vector<int32_t> best_clique_G;

    constexpr double phi = 0.9; // tunable

    // Lines 2-5: warm-up pass
    // parfor k ← |C*| to degeneracy+1:
    for (int32_t k = static_cast<int32_t>(best_clique_G.size()); k <= max_core; ++k) {
        // v ← lowest-numbered vertex with c[v] = k
        int32_t v = -1;
        for (int32_t i = 0; i < n; ++i) {
            if (core_H[static_cast<std::size_t>(i)] == k) {
                v = i;
                break;
            }
        }
        if (v >= 0) {
            NeighborSearch(H, v, core_H, best_clique_G, phi);
        }
    }

    // Lines 6-11: main pass
    for (int32_t k = max_core; k >= 1; --k) {
        if (k < static_cast<int32_t>(best_clique_G.size())) continue;

        for (int32_t v = 0; v < n; ++v) {
            if (core_H[static_cast<std::size_t>(v)] != k) continue;
            if (core_H[static_cast<std::size_t>(v)] < static_cast<int32_t>(best_clique_G.size())) continue;

            NeighborSearch(H, v, core_H, best_clique_G, phi);
        }
    }

    return best_clique_G;
}
}

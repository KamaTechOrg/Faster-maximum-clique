#pragma once

#include <cstdint>
#include <vector>

#include "lazymc/lazygraph.h"

namespace lazymc {

// Algorithm 8: search the right-neighborhood of vH (H-labeling).
// - H: LazyGraph (H-labeling)
// - vH: vertex in H-labeling
// - core_H: coreness array in H-labeling
// - best_clique_G: incumbent clique in ORIGINAL G-labeling (updated if improved)
// - phi: density threshold (0..1 typically, but your tests also use >1)
void NeighborSearch(LazyGraph& H,
                    int32_t vH,
                    const std::vector<int32_t>& core_H,
                    std::vector<int32_t>& best_clique_G,
                    double phi);

} // namespace lazymc

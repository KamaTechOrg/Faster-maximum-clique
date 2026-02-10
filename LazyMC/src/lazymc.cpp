#include "lazymc/lazymc.h"
#include "lazymc/lazygraph.h"
#include "lazymc/kcore.h"
#include "lazymc/determine_sort_order.h"
#include "lazymc/degree_heuristic.h"
#include "lazymc/coreness_heuristic.h"
#include "lazymc/systematic_search.h"

#include <cstddef>
#include <vector>
#include <cstdint>

namespace lazymc {

namespace {
// Global incumbent clique, as in Algorithm 1 (C*).
// For now it is only used inside LazyMC_run.
std::vector<int32_t> g_incumbent_clique;
} // namespace

std::vector<int32_t> LazyMC_run(const AdjList& G) {
    // Step 2: initialize incumbent clique C* = {}
    g_incumbent_clique.clear();

    // Step 3: DegreeBasedHeuristicSearch(G)
{ 
    std::vector<int32_t> clique = degree_based_heuristic(G, 36);

    if (clique.size() > g_incumbent_clique.size()) {
        g_incumbent_clique = std::move(clique);
    }
}


    // Step 4: KCore(G, |C*|)
    const int32_t clique_lb = static_cast<int32_t>(g_incumbent_clique.size());
    std::vector<int32_t> core = KCore(G, clique_lb);


    // Step 5: DetermineSortOrder(G, core)
    const SortOrder order = DetermineSortOrder(G, core);
    // Convert core from G-labeling to H-labeling
    std::vector<int32_t> core_H(order.relabel_to_original.size(), 0);
    for (std::size_t vH = 0; vH < order.relabel_to_original.size(); ++vH) {
    const int32_t vG = order.relabel_to_original[vH];
    core_H[vH] = core[static_cast<std::size_t>(vG)];
}


    // Step 6: build LazyGraph H from G, order o, and core values.
    LazyGraph H(G, order.relabel_to_original, order.original_to_relabel, core_H);

{
   // Step 7: CorenessBasedHeuristicSearch(H, core_H)

    std::vector<int32_t> clique = coreness_based_heuristic(H, core_H);
    if (clique.size() > g_incumbent_clique.size()) {
        g_incumbent_clique = std::move(clique);
    }


}

   // Step 8: SystematicSearch(H, core)
{
    std::vector<int32_t> clique = systematic_search(H, core_H);
    if (clique.size() > g_incumbent_clique.size()) {
        g_incumbent_clique = std::move(clique);
    }
}


    // For now we simply return the current incumbent clique (possibly empty).
    return g_incumbent_clique;
}

} // namespace lazymc

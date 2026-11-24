#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

namespace lazymc {

using AdjList = std::vector<std::vector<int32_t>>;

// Number of BK search-tree nodes visited (both implementations add to it).
extern std::atomic<long long> g_bk_nodes;
// Total wall time spent inside BK calls, summed over threads (ns).
extern std::atomic<long long> g_bk_ns;

// Same search as bron_kerbosch_maximum(), on the subgraph of G induced by
// verts_sorted (G-labels, ascending), using bit vectors for P, X and N(v).
// Returns a clique (G-labels) larger than initial_best_size, or empty.
std::vector<int32_t> bron_kerbosch_maximum_bitset(const AdjList& G,
                                                  const std::vector<int32_t>& verts_sorted,
                                                  int initial_best_size);

} // namespace lazymc

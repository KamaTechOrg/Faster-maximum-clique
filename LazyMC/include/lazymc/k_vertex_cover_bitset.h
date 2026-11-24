#pragma once

#include "lazymc/k_vertex_cover.h"

#include <cstdint>
#include <vector>

namespace lazymc {

// Same contract as k_vertex_cover_adj() applied to the complement of the
// subgraph of G induced by verts; cover indices are positions in verts.
// Uses bit vectors for the active set, the cover and the adjacency rows.
k_vertex_cover_res k_vertex_cover_induced_complement_bitset(const AdjList& G,
                                                            const std::vector<int32_t>& verts,
                                                            int32_t k);

} // namespace lazymc

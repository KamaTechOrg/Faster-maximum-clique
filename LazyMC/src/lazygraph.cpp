#include "lazymc/lazygraph.h"
#include <algorithm>

namespace lazymc {

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------

LazyGraph::LazyGraph(const AdjList& G,
                     const std::vector<int32_t>& relabel_to_original,
                     const std::vector<int32_t>& original_to_relabel,
                     const std::vector<int32_t>& core)
    : G_(&G),
      relabel_to_original_(relabel_to_original),
      original_to_relabel_(original_to_relabel),
      core_(core)
{
    const std::size_t n = relabel_to_original_.size();

    // Initialize flags: all zero → no neighborhoods built yet
    flags_.assign(n, static_cast<std::uint8_t>(0));

    // Allocate per-vertex containers for hashed and sorted neighborhoods
    hashed_.resize(n);
    sorted_.resize(n);
}

// ------------------------------------------------------------
// Basic information
// ------------------------------------------------------------

std::size_t LazyGraph::num_vertices() const noexcept {
    return relabel_to_original_.size();
}

const AdjList& LazyGraph::original_graph() const noexcept {
    return *G_;
}

const std::vector<int32_t>& LazyGraph::relabel_to_original() const noexcept {
    return relabel_to_original_;
}

const std::vector<int32_t>& LazyGraph::original_to_relabel() const noexcept {
    return original_to_relabel_;
}

int32_t LazyGraph::to_original(int32_t v_H) const {
    return relabel_to_original_[static_cast<std::size_t>(v_H)];
}

int32_t LazyGraph::to_relabelled(int32_t v_G) const {
    return original_to_relabel_[static_cast<std::size_t>(v_G)];
}

const std::vector<int32_t>& LazyGraph::core_values() const noexcept {
    return core_;
}

// ------------------------------------------------------------
// Neighborhood access (lazy)
// ------------------------------------------------------------

LazyGraph::NeighborHash& LazyGraph::hashed_neighbors(int32_t v) {
    const std::size_t idx = static_cast<std::size_t>(v);

    // If hashed neighbors not built yet, build them now
    if ((flags_[idx] & 0x1u) == 0u) {
        build_hashed(v);
        flags_[idx] |= 0x1u;  // Mark hashed as built
    }

    return hashed_[idx];
}

const LazyGraph::NeighborArray& LazyGraph::sorted_neighbors(int32_t v) {
    const std::size_t idx = static_cast<std::size_t>(v);

    // If sorted neighbors not built yet, build them now
    if ((flags_[idx] & 0x2u) == 0u) {
        build_sorted(v);
        flags_[idx] |= 0x2u;  // Mark sorted as built
    }

    return sorted_[idx];
}

// ------------------------------------------------------------
// Builders
// ------------------------------------------------------------

void LazyGraph::build_hashed(int32_t v) {
    const std::size_t idx = static_cast<std::size_t>(v);

    // Clear existing data (if any)
    hashed_[idx].clear();

    // Convert v from H-index to original G-index
    const int32_t v_G = relabel_to_original_[idx];

    // Access neighbors of v in the original graph
    const auto& neighbors_G = (*G_)[static_cast<std::size_t>(v_G)];

    // Convert each neighbor from G-index to H-index and store
    for (int32_t u_G : neighbors_G) {
        const std::size_t u_idx = static_cast<std::size_t>(u_G);

        // Safety check: ensure mapping is within bounds
        if (u_idx >= original_to_relabel_.size()) {
            continue;
        }

        int32_t u_H = original_to_relabel_[u_idx];

        // If mapping exists (should be valid in a consistent relabeling), insert into hash
        if (u_H >= 0) {
            hashed_[idx].insert(u_H);
        }
    }
}

void LazyGraph::build_sorted(int32_t v) {
    const std::size_t idx = static_cast<std::size_t>(v);

    // Start from a clean vector
    sorted_[idx].clear();

    // Ensure hashed neighbors are built so we can reuse them
    if ((flags_[idx] & 0x1u) == 0u) {
        build_hashed(v);
        flags_[idx] |= 0x1u;  // Mark hashed as built
    }

    // Copy neighbors from hashed_ into a vector
    sorted_[idx].assign(hashed_[idx].begin(), hashed_[idx].end());

    // Sort by H-index (degeneracy order is encoded in indices)
    std::sort(sorted_[idx].begin(), sorted_[idx].end());
}

} // namespace lazymc

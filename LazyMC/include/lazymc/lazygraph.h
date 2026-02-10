#pragma once

#include <cstdint>
#include <vector>
#include <unordered_set>
// #include <mutex>   // not needed for now
// #include <atomic>  // not needed for now

namespace lazymc {

// Original adjacency list: vertices labeled 0..n-1.
using AdjList = std::vector<std::vector<int32_t>>;

/**
 * LazyGraph
 *
 * A lazy, filtered, hashed, relabelled graph representation
 * as described in the LazyMC paper (Algorithm 2).
 *
 * For now, this implementation assumes single-threaded use.
 */
class LazyGraph {
public:
    using NeighborHash  = std::unordered_set<int32_t>;
    using NeighborArray = std::vector<int32_t>;

    LazyGraph(const AdjList& G,
              const std::vector<int32_t>& relabel_to_original,
              const std::vector<int32_t>& original_to_relabel,
              const std::vector<int32_t>& core);

    // ---------- basic information ----------

    std::size_t num_vertices() const noexcept;
    const AdjList& original_graph() const noexcept;

    const std::vector<int32_t>& relabel_to_original() const noexcept;
    const std::vector<int32_t>& original_to_relabel() const noexcept;

    int32_t to_original(int32_t v_H) const;
    int32_t to_relabelled(int32_t v_G) const;

    const std::vector<int32_t>& core_values() const noexcept;
    int32_t clique_lower_bound() const noexcept;

    // ---------- lazy neighborhood access ----------

    NeighborHash& hashed_neighbors(int32_t v);
    const NeighborArray& sorted_neighbors(int32_t v);

private:
    // Lazy builders (Algorithm 2) – to be implemented.
    void build_hashed(int32_t v);
    void build_sorted(int32_t v);

private:
    // Pointer to original graph (not owned).
    const AdjList* G_ = nullptr;

    // Reordering mappings:
    //   relabel_to_original_[v_H] = v_G
    //   original_to_relabel_[v_G] = v_H
    std::vector<int32_t> relabel_to_original_;
    std::vector<int32_t> original_to_relabel_;

    // Coreness array c[v] in H-labeling.
    std::vector<int32_t> core_;

    // Flags per vertex:
    // bit 0 (1) = hashed already created
    // bit 1 (2) = sorted already created
    // For now, single-threaded, so no atomics.
    std::vector<std::uint8_t> flags_;

    // Hashed (unordered_set) representation.
    std::vector<NeighborHash> hashed_;

    // Sorted (vector) representation.
    std::vector<NeighborArray> sorted_;

    // NOTE: no per-vertex mutexes for now; single-threaded only.
    // std::vector<std::mutex> locks_;
};

} // namespace lazymc

#include "lazymc/k_vertex_cover.h"
#include "lazymc/bk_bitset.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

using lazymc::AdjList;

constexpr long long VC_STEP_LIMIT = 20'000'000;

// Internal state for recursion.
struct VCState {
    const AdjList* G = nullptr;              // Pointer to original graph
    std::vector<std::uint8_t> active;        // 1 = vertex is still present, 0 = removed
    std::vector<int32_t> degree;             // Degree in the active graph
    int32_t k = 0;                           // Remaining allowed vertices in the cover
    std::vector<int32_t> cover;              // Vertices chosen so far
};

// ---------- basic helpers ----------

inline bool is_active(const VCState& state, int32_t v) {
    return v >= 0 &&
           v < static_cast<int32_t>(state.active.size()) &&
           state.active[static_cast<std::size_t>(v)] != 0;
}

inline int32_t degree_active(const VCState& state, int32_t v) {
    if (!is_active(state, v)) {
        return 0;
    }
    return state.degree[static_cast<std::size_t>(v)];
}

// Take a single vertex v into the cover, update k, active, and degrees.
// Returns false if k drops below 0 (prune).
bool take_vertex_into_cover(VCState& state, int32_t v) {
    if (!is_active(state, v)) {
        // Vertex already inactive: nothing to do
        return true;
    }

    state.cover.push_back(v);
    --state.k;
    if (state.k < 0) {
        return false;
    }

    state.active[static_cast<std::size_t>(v)] = 0;
    state.degree[static_cast<std::size_t>(v)] = 0;

    const auto& nbrs = (*state.G)[static_cast<std::size_t>(v)];
    for (int32_t u : nbrs) {
        if (is_active(state, u)) {
            auto idx = static_cast<std::size_t>(u);
            if (state.degree[idx] > 0) {
                --state.degree[idx];
            }
        }
    }

    return true;
}

// Take a set of vertices into the cover at once.
// Only vertices that are still active are actually taken.
// Returns false if k < 0.
bool take_vertices_into_cover(VCState& state, const std::vector<int32_t>& verts) {
    for (int32_t v : verts) {
        if (!is_active(state, v)) {
            continue; // Vertex already removed, skip
        }
        state.cover.push_back(v);
        --state.k;
        if (state.k < 0) {
            return false;
        }

        state.active[static_cast<std::size_t>(v)] = 0;
        state.degree[static_cast<std::size_t>(v)] = 0;

        const auto& nbrs = (*state.G)[static_cast<std::size_t>(v)];
        for (int32_t u : nbrs) {
            if (is_active(state, u)) {
                auto idx = static_cast<std::size_t>(u);
                if (state.degree[idx] > 0) {
                    --state.degree[idx];
                }
            }
        }
    }
    return true;
}

// Check if the active graph still has at least one edge.
bool graph_has_edges(const VCState& state) {
    const int32_t n = static_cast<int32_t>(state.active.size());
    for (int32_t v = 0; v < n; ++v) {
        if (!is_active(state, v)) {
            continue;
        }
        if (degree_active(state, v) > 0) {
            return true;
        }
    }
    return false;
}

// Validate that all neighbors in the adjacency lists are within [0, n).
inline bool validate_graph_indices(const AdjList& G) {
    const int32_t n = static_cast<int32_t>(G.size());
    for (int32_t v = 0; v < n; ++v) {
        const auto& nbrs = G[static_cast<std::size_t>(v)];
        for (int32_t u : nbrs) {
            if (u < 0 || u >= n) {
                return false;
            }
        }
    }
    return true;
}

// Greedy matching to get a lower bound on the vertex cover size.
// For any matching of size M, vc(G) >= M.
// If this bound exceeds k, we can safely prune.
int32_t greedy_matching_lower_bound(const AdjList& G,
                                    const std::vector<std::uint8_t>& active,
                                    int32_t k_for_prune) {
    const int32_t n = static_cast<int32_t>(G.size());
    std::vector<std::uint8_t> used(static_cast<std::size_t>(n), 0);
    int32_t match_count = 0;

    for (int32_t v = 0; v < n; ++v) {
        if (!active[static_cast<std::size_t>(v)] ||
            used[static_cast<std::size_t>(v)]) {
            continue;
        }
        const auto& nbrs = G[static_cast<std::size_t>(v)];
        for (int32_t u : nbrs) {
            if (!active[static_cast<std::size_t>(u)] ||
                used[static_cast<std::size_t>(u)]) {
                continue;
            }
            // Match (v, u)
            used[static_cast<std::size_t>(v)] = 1;
            used[static_cast<std::size_t>(u)] = 1;
            ++match_count;
            if (match_count > k_for_prune) {
                // We only need to know that the lower bound exceeds k
                return match_count;
            }
            break;
        }
    }

    return match_count;
}

// ---------- kernelization ----------
//
//  - Remove isolated vertices (deg = 0).
//  - Degree-1 rule: if deg(u) = 1 with neighbor v, then v must be in any cover.
//  - High-degree rule: if deg(v) > k, v must be in any k-vertex-cover.
//
// Runs until no rule applies or k < 0.
bool kernelize(VCState& state) {
    const int32_t n = static_cast<int32_t>(state.active.size());

    while (true) {
        if (state.k < 0) {
            return false;
        }

        bool changed = false;

        // 1. Remove isolated vertices (deg = 0).
        for (int32_t v = 0; v < n; ++v) {
            if (!is_active(state, v)) {
                continue;
            }
            if (degree_active(state, v) == 0) {
                state.active[static_cast<std::size_t>(v)] = 0;
                state.degree[static_cast<std::size_t>(v)] = 0;
                changed = true;
            }
        }
        if (state.k < 0) {
            return false;
        }
        if (changed) {
            continue;
        }

        // 2. Degree-1 rule: if u has exactly one active neighbor v, we must take v.
        for (int32_t u = 0; u < n; ++u) {
            if (!is_active(state, u)) {
                continue;
            }
            if (degree_active(state, u) == 1) {
                int32_t only_neighbor = -1;
                const auto& nbrs = (*state.G)[static_cast<std::size_t>(u)];
                for (int32_t v : nbrs) {
                    if (is_active(state, v)) {
                        only_neighbor = v;
                        break;
                    }
                }
                if (only_neighbor >= 0) {
                    if (!take_vertex_into_cover(state, only_neighbor)) {
                        return false;
                    }
                    changed = true;
                    break; // Restart kernelization loop
                }
            }
        }
        if (changed) {
            continue;
        }

        // 3. High-degree rule: if deg(v) > k, then v must be in every k-vertex-cover.
        for (int32_t v = 0; v < n; ++v) {
            if (!is_active(state, v)) {
                continue;
            }
            const int32_t deg_v = degree_active(state, v);
            if (deg_v > state.k) {
                if (!take_vertex_into_cover(state, v)) {
                    return false;
                }
                changed = true;
                break; // Restart kernelization loop
            }
        }

        if (!changed) {
            break; // No more rules apply
        }
    }

    return state.k >= 0;
}

// Pick a vertex of maximum degree among active vertices.
// Returns false if there are no active vertices with degree > 0.
bool pick_high_degree_vertex(const VCState& state, int32_t& v_out) {
    const int32_t n = static_cast<int32_t>(state.active.size());

    int32_t best_v = -1;
    int32_t best_deg = -1;

    for (int32_t v = 0; v < n; ++v) {
        if (!is_active(state, v)) {
            continue;
        }
        int32_t d = degree_active(state, v);
        if (d > best_deg) {
            best_deg = d;
            best_v = v;
        }
    }

    if (best_v < 0 || best_deg <= 0) {
        return false;
    }

    v_out = best_v;
    return true;
}

// ---------- main recursive solver ----------
//
// Branching rule (stronger):
//   Choose v of maximum degree.
//   - Branch A: put v into the cover (k <- k - 1).
//   - Branch B: put all neighbors of v into the cover (k <- k - deg(v)).
//
// In addition, we keep a global step counter, and if it
// exceeds VC_STEP_LIMIT, we set 'aborted = true' and stop.
//
bool solve(VCState& state,
           long long& steps,
           long long step_limit,
           bool& aborted)
{
    // Global step limit check
    if (steps > step_limit) {
        aborted = true;
        return false;
    }
    ++steps;

    // 1. Kernelization.
    if (!kernelize(state)) {
        return false; // Exceeded k during reductions
    }

    // Check step limit again after kernelization
    if (steps > step_limit) {
        aborted = true;
        return false;
    }

    // 2. If no edges remain, current cover is valid.
    if (!graph_has_edges(state)) {
        return true;
    }

    // 3. Lower bound using greedy matching.
    {
        int32_t lb = greedy_matching_lower_bound(*state.G, state.active, state.k);
        if (lb > state.k) {
            return false;
        }
    }

    // 4. Pick high-degree vertex v.
    int32_t v = -1;
    if (!pick_high_degree_vertex(state, v)) {
        // No active edges
        return true;
    }

    // Collect active neighbors of v (this is N(v) in the current subproblem).
    std::vector<int32_t> neighbors;
    {
        const auto& nbrs = (*state.G)[static_cast<std::size_t>(v)];
        neighbors.reserve(nbrs.size());
        for (int32_t u : nbrs) {
            if (is_active(state, u)) {
                neighbors.push_back(u);
            }
        }
    }

    // ----- Branch A: take v into the cover -----
    {
        VCState child = state;
        if (take_vertex_into_cover(child, v)) {
            if (solve(child, steps, step_limit, aborted)) {
                if (!aborted) {
                    state.cover.swap(child.cover);
                    return true;
                } else {
                    return false;
                }
            }
        }
        if (aborted) {
            return false;
        }
    }

    // ----- Branch B: take all neighbors of v into the cover -----
    {
        VCState child = state;

        if (static_cast<int32_t>(neighbors.size()) <= child.k) {
            if (take_vertices_into_cover(child, neighbors)) {
                if (solve(child, steps, step_limit, aborted)) {
                    if (!aborted) {
                        state.cover.swap(child.cover);
                        return true;
                    } else {
                        return false;
                    }
                }
            }
        }
        if (aborted) {
            return false;
        }
    }

    return false;
}

} // anonymous namespace

namespace lazymc {

k_vertex_cover_res k_vertex_cover_adj(const AdjList& G, int32_t k) {
    k_vertex_cover_res result;

    if (k < 0) {
        result.found = false;
        result.aborted = false;
        return result;
    }

    const int32_t n = static_cast<int32_t>(G.size());

    // Validate that all adjacency indices are within [0, n).
    if (!validate_graph_indices(G)) {
        result.found = false;
        result.aborted = true; // Invalid input detected
        return result;
    }

    VCState state;
    state.G = &G;
    state.k = k;
    state.active.assign(static_cast<std::size_t>(n), 1u);
    state.degree.assign(static_cast<std::size_t>(n), 0);

    // Initialize degrees in the original graph.
    for (int32_t v = 0; v < n; ++v) {
        const auto& nbrs = G[static_cast<std::size_t>(v)];
        int32_t deg = 0;
        for (int32_t u : nbrs) {
            // After validate_graph_indices, all u are in [0, n),
            // so we can safely count all neighbors.
            ++deg;
        }
        state.degree[static_cast<std::size_t>(v)] = deg;
    }

    // Global lower bound on the whole graph before recursion.
    {
        int32_t lb = greedy_matching_lower_bound(G, state.active, k);
        if (lb > state.k) {
            result.found = false;
            result.aborted = false;
            return result;
        }
    }

    long long steps = 0;
    bool aborted = false;

    const bool ok = solve(state, steps, VC_STEP_LIMIT, aborted);
    g_kvc_steps.fetch_add(steps, std::memory_order_relaxed);
    if (aborted) g_kvc_aborts.fetch_add(1, std::memory_order_relaxed);

    result.aborted = aborted;

    if (!ok || aborted) {
        result.found = false;
        return result;
    }

    result.found = true;
    result.cover = std::move(state.cover);
    return result;
}

} // namespace lazymc

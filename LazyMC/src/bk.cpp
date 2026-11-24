#include "lazymc/bk.h"
#include "lazymc/incumbent.h"
#include "lazymc/bk_bitset.h"

#include <algorithm> // std::find, std::binary_search, std::min
#include <vector>
#include <map>

namespace lazymc {

namespace {

// Return neighbors of v if exist, otherwise an empty vector we pass in.
const std::vector<int>& neighbors_or_empty(const AdjMap& G,
                                           int v,
                                           const std::vector<int>& empty) {
    auto it = G.find(v);
    if (it == G.end()) {
        return empty;
    }
    return it->second;
}

// Assuming vec is sorted: check if it contains x using binary search.
bool contains_sorted(const std::vector<int>& vec, int x) {
    return std::binary_search(vec.begin(), vec.end(), x);
}

// Intersection of two *sorted* vectors a and b, in O(|a| + |b|).
std::vector<int> intersect_sorted(const std::vector<int>& a,
                                  const std::vector<int>& b) {
    std::vector<int> out;
    out.reserve(std::min(a.size(), b.size()));

    auto itA = a.begin();
    auto itB = b.begin();

    while (itA != a.end() && itB != b.end()) {
        if (*itA < *itB) {
            ++itA;
        } else if (*itB < *itA) {
            ++itB;
        } else { // equal
            out.push_back(*itA);
            ++itA;
            ++itB;
        }
    }
    return out;
}

// ========================= BK1: all maximal cliques =========================

void bron_kerbosch_recursive_all(const AdjMap& G,
                                 std::vector<int>& R,
                                 std::vector<int>& P,
                                 std::vector<int>& X,
                                 std::vector<std::vector<int>>& cliques) {
    if (P.empty() && X.empty()) {
        cliques.push_back(R);
        return;
    }

    std::vector<int> empty_neighbors;
    std::vector<int> P_copy = P; // we may modify P, so iterate on a copy

    for (int v : P_copy) {
        R.push_back(v);

        const auto& Nv = neighbors_or_empty(G, v, empty_neighbors);
        std::vector<int> P_next = intersect_sorted(P, Nv);
        std::vector<int> X_next = intersect_sorted(X, Nv);

        bron_kerbosch_recursive_all(G, R, P_next, X_next, cliques);

        R.pop_back();

        auto itP = std::find(P.begin(), P.end(), v);
        if (itP != P.end()) {
            P.erase(itP);
        }
        X.push_back(v);
    }
}

// ===================== BK2: maximum clique with pivot =======================

// Choose pivot u from P ∪ X that maximises |N(u) ∩ P|.
int choose_pivot(const AdjMap& G,
                 const std::vector<int>& P,
                 const std::vector<int>& X) {
    int pivot = -1;
    std::size_t max_deg = 0;

    std::vector<int> empty_neighbors;

    auto consider_set = [&](const std::vector<int>& S) {
        for (int u : S) {
            const auto& Nu = neighbors_or_empty(G, u, empty_neighbors);
            std::size_t cnt = 0;
            for (int v : P) {
                if (contains_sorted(Nu, v)) {
                    ++cnt;
                }
            }
            if (cnt > max_deg) {
                max_deg = cnt;
                pivot = u;
            }
        }
    };

    consider_set(P);
    consider_set(X);

    return pivot;
}

// Greedy coloring of vertices in P to get upper bound on clique size.
// Returns the number of colors used (which is an upper bound on max clique in G[P]).
std::size_t greedy_color_bound(const AdjMap& G, const std::vector<int>& P) {
    if (P.empty()) return 0;
    
    std::vector<int> color(P.size(), -1);
    std::size_t num_colors = 0;
    
    // Map from vertex to index in P
    std::map<int, std::size_t> vertex_to_idx;
    for (std::size_t i = 0; i < P.size(); ++i) {
        vertex_to_idx[P[i]] = i;
    }
    
    std::vector<int> empty_neighbors;
    
    for (std::size_t i = 0; i < P.size(); ++i) {
        int v = P[i];
        const auto& Nv = neighbors_or_empty(G, v, empty_neighbors);
        
        // Find colors used by neighbors in P
        std::vector<bool> used_colors(num_colors + 1, false);
        for (int u : Nv) {
            auto it = vertex_to_idx.find(u);
            if (it != vertex_to_idx.end() && color[it->second] >= 0) {
                used_colors[static_cast<std::size_t>(color[it->second])] = true;
            }
        }
        
        // Find smallest available color
        int c = 0;
        while (static_cast<std::size_t>(c) < used_colors.size() && used_colors[static_cast<std::size_t>(c)]) {
            ++c;
        }
        
        color[i] = c;
        if (static_cast<std::size_t>(c) >= num_colors) {
            num_colors = static_cast<std::size_t>(c) + 1;
        }
    }
    
    return num_colors;
}

// Bron–Kerbosch with pivot heuristic, searching only for a maximum clique.
void bron_kerbosch_recursive_pivot_max(const AdjMap& G,
                                       std::vector<int>& R,
                                       std::vector<int>& P,
                                       std::vector<int>& X,
                                       std::vector<int>& best) {
    g_bk_nodes.fetch_add(1, std::memory_order_relaxed);
    // Use global incumbent for tighter pruning
    const std::size_t global_best = static_cast<std::size_t>(g_best_size.load(std::memory_order_relaxed));
    const std::size_t current_best = std::max(best.size(), global_best);
    
    // Quick bound: even if we add all of P to R, can't beat best.
    if (R.size() + P.size() <= current_best) {
        return;
    }
    
    // Tighter bound using greedy coloring (only if P is not too large)
    if (P.size() <= 100) {
        std::size_t color_bound = greedy_color_bound(G, P);
        if (R.size() + color_bound <= current_best) {
            return;
        }
    }

    if (P.empty() && X.empty()) {
        if (R.size() > best.size()) {
            best = R;
            // Update global incumbent for cross-branch pruning
            if (R.size() > global_best) {
                std::vector<int32_t> clique_copy(R.begin(), R.end());
                try_update_incumbent(std::move(clique_copy));
            }
        }
        return;
    }

    int pivot = choose_pivot(G, P, X);

    std::vector<int> empty_neighbors;
    const auto& Nu_pivot = (pivot == -1)
        ? empty_neighbors
        : neighbors_or_empty(G, pivot, empty_neighbors);

    // Candidates: P \ N(pivot)
    std::vector<int> candidates;
    candidates.reserve(P.size());
    if (pivot == -1) {
        candidates = P; // no pivot chosen, fall back to plain P
    } else {
        for (int v : P) {
            if (!contains_sorted(Nu_pivot, v)) {
                candidates.push_back(v);
            }
        }
    }

    for (int v : candidates) {
        R.push_back(v);

        const auto& Nv = neighbors_or_empty(G, v, empty_neighbors);
        std::vector<int> P_next = intersect_sorted(P, Nv);
        std::vector<int> X_next = intersect_sorted(X, Nv);

        bron_kerbosch_recursive_pivot_max(G, R, P_next, X_next, best);

        R.pop_back();

        // Move v from P to X
        auto itP = std::find(P.begin(), P.end(), v);
        if (itP != P.end()) {
            P.erase(itP);
        }
        X.push_back(v);
    }
}

} // anonymous namespace

// ========================== Public API: all cliques =========================

std::vector<std::vector<int>> bron_kerbosch_all(const AdjMap& G) {
    std::vector<std::vector<int>> cliques;

    std::vector<int> P;
    P.reserve(G.size());
    for (const auto& [v, _] : G) {
        P.push_back(v);
    }

    // נוודא ש-P ממוינת, כדי שהחיתוכים יעבדו ביעילות
    std::sort(P.begin(), P.end());

    std::vector<int> R; // current clique
    std::vector<int> X; // excluded vertices

    bron_kerbosch_recursive_all(G, R, P, X, cliques);
    return cliques;
}

// ======================= Public API: maximum clique ========================

std::vector<int> bron_kerbosch_maximum(const AdjMap& G, int initial_best_size) {
    std::vector<int> best;

    if (G.empty()) {
        return best; // no vertices, no clique
    }

    // Early prune: if graph can't possibly beat initial_best_size, return empty
    if (static_cast<int>(G.size()) <= initial_best_size) {
        return best;
    }

    // Initialize best with a dummy size so pruning kicks in immediately
    // We use resize to set the size without actual vertices (they won't be returned)
    best.resize(static_cast<std::size_t>(initial_best_size));

    std::vector<int> P;
    P.reserve(G.size());
    for (const auto& [v, _] : G) {
        P.push_back(v);
    }

    // Keep P sorted for efficient intersection
    std::sort(P.begin(), P.end());

    std::vector<int> R; // current clique
    std::vector<int> X; // excluded vertices

    bron_kerbosch_recursive_pivot_max(G, R, P, X, best);

    // If we only have the dummy, no better clique was found
    if (!best.empty() && best[0] == 0 && static_cast<int>(best.size()) == initial_best_size) {
        // Check if it's the dummy (all zeros)
        bool is_dummy = true;
        for (int v : best) {
            if (v != 0) { is_dummy = false; break; }
        }
        if (is_dummy && initial_best_size > 0) {
            best.clear();
        }
    }

    return best;
}

} // namespace lazymc

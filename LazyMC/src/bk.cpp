#include "lazymc/bk.h"

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

// Bron–Kerbosch with pivot heuristic, searching only for a maximum clique.
void bron_kerbosch_recursive_pivot_max(const AdjMap& G,
                                       std::vector<int>& R,
                                       std::vector<int>& P,
                                       std::vector<int>& X,
                                       std::vector<int>& best) {
    // Simple bound: even if we add all of P to R, can't beat best.
    if (R.size() + P.size() <= best.size()) {
        return;
    }

    if (P.empty() && X.empty()) {
        if (R.size() > best.size()) {
            best = R;
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

    // Iterate over a copy, but we will update P and X along the way.
    std::vector<int> candidates_copy = candidates;

    for (int v : candidates_copy) {
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

std::vector<int> bron_kerbosch_maximum(const AdjMap& G) {
    std::vector<int> best;

    if (G.empty()) {
        return best; // no vertices, no clique
    }

    std::vector<int> P;
    P.reserve(G.size());
    for (const auto& [v, _] : G) {
        P.push_back(v);
    }

    std::sort(P.begin(), P.end());

    std::vector<int> R; // current clique
    std::vector<int> X; // excluded vertices

    bron_kerbosch_recursive_pivot_max(G, R, P, X, best);
    return best;
}

} // namespace lazymc

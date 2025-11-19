// candidate_ops.cpp
#include "lazymc/core/candidate_ops.hpp"
#include "lazymc/core/intersect.hpp"
#include "lazymc/graph/graph.hpp"

namespace lazymc {

// פונקציות גלובליות של LazyMC

/// N ← N ∩ NG(u), כולל עדכון inN
void filter_N(
    std::vector<int>& N,
    std::vector<char>& inN,
    int u,
    const graph::Graph& G
) {
    // קבלת שכני u
    const auto& neighbors_u = G.neighbors(u);
    
    // נקה את inN (רק עבור הקודקודים הישנים ב-N)
    for (int v : N) {
        inN[v] = 0;
    }
    
    // בצע חיתוך: N ← N ∩ neighbors(u)
    N = core::Intersect::hash_intersection(N, neighbors_u);
    
    // עדכן את inN לפי ה-N החדש
    for (int v : N) {
        inN[v] = 1;
    }
}

/// inN[v] = 1 אם v ב-N
void rebuild_inN(std::vector<char>& inN,
                 const std::vector<int>& N) {
    // נקה את כל inN
    std::fill(inN.begin(), inN.end(), 0);
    
    // סמן את כל הקודקודים ב-N
    for (int v : N) {
        if (v >= 0 && v < static_cast<int>(inN.size())) {
            inN[v] = 1;
        }
    }
}

namespace core {

bool CandidateOps::is_clique(const std::vector<std::vector<int>>& adj, 
                             const Candidate& vertices) {
    // TODO: Implement
    return false;
}

CandidateOps::Candidate CandidateOps::extend(const Candidate& clique, int vertex) {
    Candidate result = clique;
    result.push_back(vertex);
    return result;
}

} // namespace core
} // namespace lazymc

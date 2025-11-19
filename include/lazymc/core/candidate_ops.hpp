// candidate_ops.hpp
#pragma once

#include <vector>

namespace lazymc {

// Forward declaration
namespace graph {
    class Graph;
}

/// N ← N ∩ NG(u), כולל עדכון inN
void filter_N(
    std::vector<int>& N,
    std::vector<char>& inN,
    int u,
    const graph::Graph& G
);

/// inN[v] = 1 אם v ב-N
void rebuild_inN(std::vector<char>& inN,
                 const std::vector<int>& N);

namespace core {

/// \brief פעולות על מועמדים לקליקה
class CandidateOps {
public:
    using Candidate = std::vector<int>;
    
    /// \brief בדיקה האם קבוצת קודקודים יוצרת קליקה
    static bool is_clique(const std::vector<std::vector<int>>& adj, 
                         const Candidate& vertices);
    
    /// \brief הרחבת קליקה עם קודקוד חדש
    static Candidate extend(const Candidate& clique, int vertex);
};

} // namespace core
} // namespace lazymc

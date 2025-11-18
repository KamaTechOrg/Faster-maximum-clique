// candidate_ops.hpp
#pragma once

#include <vector>

namespace lazymc {
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

// coreness_heuristic.hpp
#pragma once

#include <vector>

namespace lazymc {
namespace heuristics {

/// \brief היוריסטיקות מבוססות coreness
class CorenessHeuristic {
public:
    /// \brief בחירת קודקודים לפי coreness גבוה
    static std::vector<int> select_by_high_coreness(
        const std::vector<std::vector<int>>& adj,
        int k);
    
    /// \brief הערכת גבול עליון לפי coreness
    static int estimate_upper_bound(
        const std::vector<std::vector<int>>& adj);
};

} // namespace heuristics
} // namespace lazymc

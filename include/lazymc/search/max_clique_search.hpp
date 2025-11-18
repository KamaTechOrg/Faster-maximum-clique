// max_clique_search.hpp
#pragma once

#include <vector>

namespace lazymc {
namespace search {

/// \brief ממשק ראשי לחיפוש קליקה מקסימלית
class MaxCliqueSearch {
public:
    /// \brief מציאת קליקה מקסימלית עם אלגוריתם LazyMC
    static std::vector<int> find_max_clique(
        const std::vector<std::vector<int>>& adj);
    
    /// \brief מציאת קליקה מקסימלית עם timeout
    static std::vector<int> find_max_clique_timed(
        const std::vector<std::vector<int>>& adj,
        double timeout_seconds);
    
    /// \brief מציאת כל הקליקות המקסימליות
    static std::vector<std::vector<int>> find_all_max_cliques(
        const std::vector<std::vector<int>>& adj);
};

} // namespace search
} // namespace lazymc

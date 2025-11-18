// neighbor_search.hpp
#pragma once

#include <vector>

namespace lazymc {
namespace search {

/// \brief חיפוש מבוסס שכנים
class NeighborSearch {
public:
    /// \brief חיפוש בשכונה של קודקוד
    static std::vector<int> search_neighborhood(
        const std::vector<std::vector<int>>& adj,
        int seed_vertex);
    
    /// \brief הרחבת קליקה נוכחית
    static std::vector<int> expand_clique(
        const std::vector<std::vector<int>>& adj,
        const std::vector<int>& initial_clique);
};

} // namespace search
} // namespace lazymc

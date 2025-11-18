// systematic_search.hpp
#pragma once

#include <vector>

namespace lazymc {
namespace search {

/// \brief חיפוש שיטתי (branch and bound) לקליקה מקסימלית
class SystematicSearch {
public:
    /// \brief מציאת קליקה מקסימלית באמצעות backtracking
    static std::vector<int> find_maximum_clique(
        const std::vector<std::vector<int>>& adj);
    
    /// \brief חיפוש עם גיזום (pruning)
    static std::vector<int> search_with_pruning(
        const std::vector<std::vector<int>>& adj,
        int lower_bound);

private:
    static void backtrack(
        const std::vector<std::vector<int>>& adj,
        std::vector<int>& current,
        std::vector<int>& candidates,
        std::vector<int>& best);
};

} // namespace search
} // namespace lazymc

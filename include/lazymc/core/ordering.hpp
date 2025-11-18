// ordering.hpp
#pragma once

#include <vector>

namespace lazymc {
namespace core {

/// \brief אסטרטגיות למיון קודקודים
class Ordering {
public:
    /// \brief מיון לפי דרגה (גבוה לנמוך)
    static std::vector<int> by_degree(
        const std::vector<std::vector<int>>& adj);
    
    /// \brief מיון לפי coreness
    static std::vector<int> by_coreness(
        const std::vector<std::vector<int>>& adj);
    
    /// \brief מיון דינמי במהלך החיפוש
    static std::vector<int> dynamic_ordering(
        const std::vector<std::vector<int>>& adj,
        const std::vector<int>& candidates);
};

} // namespace core
} // namespace lazymc

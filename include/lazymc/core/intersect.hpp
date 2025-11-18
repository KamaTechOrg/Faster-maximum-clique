// intersect.hpp
#pragma once

#include <vector>
#include <algorithm>

namespace lazymc {
namespace core {

/// \brief פעולות חיתוך על קבוצות קודקודים
class Intersect {
public:
    /// \brief חיתוך בין שתי רשימות ממויינות
    static std::vector<int> sorted_intersection(
        const std::vector<int>& a, 
        const std::vector<int>& b);
    
    /// \brief חיתוך מהיר עם bitset (לגרפים קטנים)
    static std::vector<int> bitset_intersection(
        const std::vector<int>& a, 
        const std::vector<int>& b,
        int max_vertex);
};

} // namespace core
} // namespace lazymc

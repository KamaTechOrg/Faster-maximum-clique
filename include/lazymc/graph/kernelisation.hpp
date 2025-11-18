// kernelisation.hpp
#pragma once

#include "graph.hpp"

namespace lazymc {
namespace graph {

/// \brief פונקציות להפחתת גודל הגרף (graph reduction/kernelisation)
class Kernelisation {
public:
    /// \brief הסרת קודקודים עם דרגה נמוכה מדי
    static Graph reduce_by_min_degree(const Graph& g, int min_degree);
    
    // TODO: Add more kernelisation techniques
};

} // namespace graph
} // namespace lazymc

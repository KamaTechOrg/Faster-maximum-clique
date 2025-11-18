// lazy_graph.hpp
#pragma once

#include "graph.hpp"

namespace lazymc {
namespace graph {

/// \brief גרסה "עצלה" של גרף עם אופטימיזציות
class LazyGraph : public Graph {
public:
    LazyGraph() = default;
    explicit LazyGraph(int num_vertices);
    
    // TODO: Add lazy evaluation optimizations
};

} // namespace graph
} // namespace lazymc

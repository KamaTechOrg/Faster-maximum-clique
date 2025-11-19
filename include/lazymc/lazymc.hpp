// lazymc.hpp
#pragma once

// Graph components
#include "lazymc/graph/graph.hpp"
#include "lazymc/graph/lazy_graph.hpp"
#include "lazymc/graph/kernelisation.hpp"

// Core algorithms
#include "lazymc/core/candidate_ops.hpp"
#include "lazymc/core/intersect.hpp"
#include "lazymc/core/k_core.hpp"
#include "lazymc/core/ordering.hpp"

// Heuristics
#include "lazymc/heuristics/degree_heuristic.hpp"
#include "lazymc/heuristics/coreness_heuristic.hpp"

// Search algorithms
#include "lazymc/search/systematic_search.hpp"
#include "lazymc/search/neighbor_search.hpp"
#include "lazymc/search/max_clique_search.hpp"

/// \brief LazyMC - Lazy Maximum Clique algorithm
/// 
/// ספרייה למציאת קליקה מקסימלית בגרפים עם אופטימיזציות "עצלות"
namespace lazymc {

/// \brief מציאת קליקה מקסימלית בגרף
/// \param adj רשימת שכנויות
/// \return וקטור של אינדקסי קודקודים בקליקה המקסימלית

struct LazyMCConfig {
    int K_top_degree = 64;
    double phi_density = 0.5; // הפרמטר φ מאלגוריתם 8
    // פרמטרים נוספים בעתיד...
};

struct LazyMCResult {
    std::vector<int> clique;
    double runtime_seconds;
};

LazyMCResult run(const graph::Graph& G, const LazyMCConfig& cfg);

/// \brief פונקציה פשוטה למציאת קליקה מקסימלית (לשימוש ב-CLI)
/// \param adj רשימת שכנויות
/// \return וקטור של אינדקסי קודקודים בקליקה המקסימלית
std::vector<int> find_maximum_clique(const std::vector<std::vector<int>>& adj);

} // namespace lazymc

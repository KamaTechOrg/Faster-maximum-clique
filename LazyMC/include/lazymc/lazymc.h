#pragma once

#include <vector>
#include <cstdint>

namespace lazymc {

/**
 * @brief Result structure for maximum clique computation
 * 
 * Contains the maximum clique found and performance metrics.
 */
struct MCResult {
    std::vector<int32_t> clique;  ///< The maximum clique found
    int64_t nodes = 0;             ///< Number of search nodes explored
    double ms = 0.0;               ///< Computation time in milliseconds
};

/**
 * @brief Main entry point for LazyMC algorithm
 * 
 * Finds the maximum clique in an undirected graph using the LazyMC algorithm,
 * which combines:
 * 1. Degree-based heuristic search (Algorithm 5)
 * 2. K-core decomposition for vertex ordering
 * 3. Coreness-based heuristic seeds (Algorithm 6)
 * 4. Systematic branch-and-bound search (Algorithm 7)
 * 
 * @param G Adjacency list representation of the graph where G[v] contains
 *          the list of neighbors of vertex v
 * @return MCResult containing the maximum clique and performance metrics
 * 
 * Algorithm phases:
 * - Phase 1: Quick heuristic search using degree-based expansion
 * - Phase 2: Compute k-core decomposition and order vertices
 * - Phase 3: Generate seeds using coreness-based heuristic
 * - Phase 4: Systematic search with branch-and-bound
 * 
 * @note Graph vertices should be numbered 0 to n-1
 * @note The graph should be simple (no self-loops or multi-edges)
 */
MCResult run_lazymc(const std::vector<std::vector<int32_t>>& G);

/**
 * @brief Global incumbent clique (best solution found so far)
 * 
 * This is shared across all algorithm components to enable pruning
 * based on the current best solution.
 * 
 * @note In multi-threaded implementations, this should be protected with mutex
 */
extern std::vector<int32_t> global_incumbent_clique;

} // namespace lazymc

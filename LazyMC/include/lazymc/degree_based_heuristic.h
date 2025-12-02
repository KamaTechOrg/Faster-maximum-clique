#pragma once

#include <vector>
#include <cstdint>

namespace lazymc {

/**
 * @brief Degree-based heuristic search for maximum clique (Algorithm 5)
 * 
 * This algorithm implements the degree-based heuristic search from the LazyMC paper.
 * It selects the top-K vertices with highest degree and attempts to build a maximal
 * clique starting from each vertex using a greedy expansion strategy.
 * 
 * @param G Adjacency list representation of the graph
 * @param top_k Number of highest-degree vertices to consider as starting points
 * @return The best clique found
 * 
 * Algorithm steps:
 * 1. Identify top-K vertices with highest degree
 * 2. For each top-K vertex v:
 *    a. Initialize candidate set N with neighbors of v that have degree ≥ |C*|
 *    b. Initialize clique C = {v}
 *    c. Greedily expand: repeatedly add vertex u from N with maximum |N ∩ N_G(u)|
 *    d. Update N = N ∩ N_G(u) after each addition
 * 3. Return the largest clique found
 */
std::vector<int32_t> degree_based_heuristic(
    const std::vector<std::vector<int32_t>>& G, 
    int top_k);

} // namespace lazymc

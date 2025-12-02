#pragma once

#include <vector>
#include <cstdint>

namespace lazymc {

/**
 * @brief Neighbor search utilities for clique expansion
 * 
 * This module provides efficient algorithms for finding candidate vertices
 * during clique expansion and computing neighborhood intersections.
 * 
 * Key operations:
 * - Finding common neighbors of a set of vertices
 * - Computing neighborhood intersection sizes efficiently
 * - Filtering candidates based on degree constraints
 */

/**
 * @brief Find common neighbors of all vertices in a clique
 * 
 * @param G Adjacency list representation of the graph
 * @param clique Current clique vertices
 * @return Vector of vertices that are neighbors of all clique members
 */
std::vector<int32_t> find_common_neighbors(
    const std::vector<std::vector<int32_t>>& G,
    const std::vector<int32_t>& clique);

/**
 * @brief Compute size of intersection between candidate set and vertex neighbors
 * 
 * @param G Adjacency list representation
 * @param candidates Candidate vertex set
 * @param vertex Target vertex
 * @return Number of candidates that are neighbors of vertex
 */
int32_t compute_intersection_size(
    const std::vector<std::vector<int32_t>>& G,
    const std::vector<int32_t>& candidates,
    int32_t vertex);

} // namespace lazymc

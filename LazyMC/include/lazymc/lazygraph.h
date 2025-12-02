#pragma once

#include <vector>
#include <cstdint>

namespace lazymc {

/**
 * @brief LazyGraph - Efficient graph representation with lazy k-core computation
 * 
 * This class implements the core data structure for the LazyMC algorithm.
 * It maintains the graph structure and computes k-core values on demand
 * during the search process.
 * 
 * Key features:
 * - Adjacency list representation
 * - Lazy k-core computation (computed only when needed)
 * - Efficient neighbor intersection operations
 * - Support for induced subgraphs
 * 
 * @note Currently not implemented as a class - functionality is distributed
 */
class LazyGraph {
public:
    /**
     * @brief Construct a LazyGraph from an adjacency list
     * @param adj_list Adjacency list representation
     */
    explicit LazyGraph(const std::vector<std::vector<int32_t>>& adj_list);
    
    /**
     * @brief Get the k-core number for a vertex
     * @param v Vertex ID
     * @return k-core number of vertex v
     */
    int32_t get_core_number(int32_t v) const;
    
    /**
     * @brief Compute k-core decomposition lazily
     * @param vertices Subset of vertices to process
     */
    void compute_cores(const std::vector<int32_t>& vertices);
    
    /**
     * @brief Get neighbors of a vertex
     * @param v Vertex ID
     * @return Vector of neighbor IDs
     */
    const std::vector<int32_t>& get_neighbors(int32_t v) const;
    
private:
    std::vector<std::vector<int32_t>> adj_list_;
    std::vector<int32_t> core_numbers_;
    std::vector<bool> core_computed_;
};

} // namespace lazymc

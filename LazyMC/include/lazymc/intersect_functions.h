#pragma once

#include <vector>
#include <cstdint>

namespace lazymc {

/**
 * @brief Set intersection utilities for efficient neighbor computations
 * 
 * This module provides optimized set intersection algorithms used throughout
 * the LazyMC algorithm for computing common neighbors and pruning candidates.
 * 
 * Implementation strategies:
 * - Hash-based intersection for unsorted sets
 * - Merge-based intersection for sorted sets
 * - Adaptive selection based on set sizes
 */

/**
 * @brief Compute intersection of two sorted vectors
 * 
 * @param a First sorted vector
 * @param b Second sorted vector
 * @return Vector containing elements in both a and b
 * 
 * @complexity O(|a| + |b|) time, O(min(|a|, |b|)) space
 */
std::vector<int32_t> intersect_sorted(
    const std::vector<int32_t>& a,
    const std::vector<int32_t>& b);

/**
 * @brief Compute intersection using hash set (for unsorted data)
 * 
 * @param a First vector
 * @param b Second vector
 * @return Vector containing elements in both a and b
 * 
 * @complexity O(|a| + |b|) average time, O(|a|) space
 */
std::vector<int32_t> intersect_hash(
    const std::vector<int32_t>& a,
    const std::vector<int32_t>& b);

/**
 * @brief Compute intersection size without materializing result
 * 
 * @param a First vector (should be smaller set for efficiency)
 * @param b Second vector
 * @return Number of common elements
 * 
 * @complexity O(|a| * log|b|) if using binary search, O(|a| + |b|) with hash
 */
int32_t intersect_size(
    const std::vector<int32_t>& a,
    const std::vector<int32_t>& b);

} // namespace lazymc

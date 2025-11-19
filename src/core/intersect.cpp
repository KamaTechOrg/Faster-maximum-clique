// intersect.cpp
#include "lazymc/core/intersect.hpp"
#include <iterator>
#include <unordered_set>

namespace lazymc {
namespace core {

std::vector<int> Intersect::sorted_intersection(
    const std::vector<int>& a, 
    const std::vector<int>& b) 
{
    std::vector<int> result;
    std::set_intersection(a.begin(), a.end(),
                         b.begin(), b.end(),
                         std::back_inserter(result));
    return result;
}

std::vector<int> Intersect::bitset_intersection(
    const std::vector<int>& a, 
    const std::vector<int>& b,
    int max_vertex) 
{
    // TODO: Implement bitset version for large graphs
    // 
    // Bitset intersection is much faster for large dense graphs:
    // 1. Create two bitsets of size max_vertex
    // 2. Set bits for elements in a and b
    // 3. Perform bitwise AND operation
    // 4. Extract set bits back to vector
    // 
    // Complexity: O(max_vertex / 64) - very fast for dense graphs
    // Memory: O(max_vertex / 8) bytes for each bitset
    // 
    // Use case: When max_vertex is small (< 10000) and graph is dense
    // 
    // Example implementation:
    //   std::bitset<MAX_VERTICES> bitset_a, bitset_b;
    //   for (int v : a) bitset_a.set(v);
    //   for (int v : b) bitset_b.set(v);
    //   auto result_bits = bitset_a & bitset_b;
    //   // Extract set bits...
    
    return sorted_intersection(a, b);
}

std::vector<int> Intersect::hash_intersection(
    const std::vector<int>& a,
    const std::vector<int>& b) 
{
    // TODO: Future optimizations:
    // 1. Switch to bitset_intersection when max_vertex is known and small
    // 2. If b is much smaller than a, iterate over b instead
    // 3. Consider using std::vector<bool> for very large vertex IDs
    // 4. For sorted inputs, use sorted_intersection (O(n+m) without hash overhead)
    //
    // Current implementation: O(|a| + |b|) time, O(|b|) space
    // Bitset alternative: O(max_vertex/64) time for dense graphs
    
    // יצירת hash set מ-b לחיפוש מהיר O(1)
    std::unordered_set<int> set_b(b.begin(), b.end());
    
    // חיתוך: שמור רק אלמנטים מ-a שגם ב-b
    std::vector<int> result;
    result.reserve(std::min(a.size(), b.size()));
    
    for (int elem : a) {
        if (set_b.count(elem) > 0) {
            result.push_back(elem);
        }
    }
    
    return result;
}

} // namespace core
} // namespace lazymc

// ordering.cpp
#include "lazymc/core/ordering.hpp"
#include <numeric>
#include <algorithm>

namespace lazymc {
namespace core {

std::vector<int> Ordering::by_degree(
    const std::vector<std::vector<int>>& adj) 
{
    int n = static_cast<int>(adj.size());
    std::vector<int> vertices(n);
    std::iota(vertices.begin(), vertices.end(), 0);
    
    std::sort(vertices.begin(), vertices.end(),
        [&](int a, int b) {
            return adj[a].size() > adj[b].size();
        });
    
    return vertices;
}

std::vector<int> Ordering::by_coreness(
    const std::vector<std::vector<int>>& adj) 
{
    // TODO: Implement ordering by coreness
    return by_degree(adj);
}

std::vector<int> Ordering::dynamic_ordering(
    const std::vector<std::vector<int>>& adj,
    const std::vector<int>& candidates) 
{
    // TODO: Implement dynamic ordering
    return candidates;
}

} // namespace core
} // namespace lazymc

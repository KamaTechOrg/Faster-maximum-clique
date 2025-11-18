// coreness_heuristic.cpp
#include "lazymc/heuristics/coreness_heuristic.hpp"
#include "lazymc/core/k_core.hpp"

namespace lazymc {
namespace heuristics {

std::vector<int> CorenessHeuristic::select_by_high_coreness(
    const std::vector<std::vector<int>>& adj,
    int k) 
{
    // TODO: Implement
    return {};
}

int CorenessHeuristic::estimate_upper_bound(
    const std::vector<std::vector<int>>& adj) 
{
    // TODO: Implement
    return static_cast<int>(adj.size());
}

} // namespace heuristics
} // namespace lazymc

// max_clique_search.cpp
#include "lazymc/search/max_clique_search.hpp"
#include "lazymc/search/systematic_search.hpp"

namespace lazymc {
namespace search {

std::vector<int> MaxCliqueSearch::find_max_clique(
    const std::vector<std::vector<int>>& adj) 
{
    return SystematicSearch::find_maximum_clique(adj);
}

std::vector<int> MaxCliqueSearch::find_max_clique_timed(
    const std::vector<std::vector<int>>& adj,
    double timeout_seconds) 
{
    // TODO: Implement with timeout
    return find_max_clique(adj);
}

std::vector<std::vector<int>> MaxCliqueSearch::find_all_max_cliques(
    const std::vector<std::vector<int>>& adj) 
{
    // TODO: Implement finding all maximal cliques
    auto max_clique = find_max_clique(adj);
    return {max_clique};
}

} // namespace search
} // namespace lazymc

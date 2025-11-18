// intersect.cpp
#include "lazymc/core/intersect.hpp"
#include <iterator>

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
    // TODO: Implement bitset version
    return sorted_intersection(a, b);
}

} // namespace core
} // namespace lazymc

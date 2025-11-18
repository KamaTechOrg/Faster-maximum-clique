// candidate_ops.cpp
#include "lazymc/core/candidate_ops.hpp"

namespace lazymc {
namespace core {

bool CandidateOps::is_clique(const std::vector<std::vector<int>>& adj, 
                             const Candidate& vertices) {
    // TODO: Implement
    return false;
}

CandidateOps::Candidate CandidateOps::extend(const Candidate& clique, int vertex) {
    Candidate result = clique;
    result.push_back(vertex);
    return result;
}

} // namespace core
} // namespace lazymc

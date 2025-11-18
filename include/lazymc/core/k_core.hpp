// k_core.hpp
#pragma once

#include <vector>

namespace lazymc {
namespace core {

/// \brief חישוב k-core decomposition
class KCore {
public:
    /// \brief חישוב coreness של כל קודקוד
    static std::vector<int> compute_coreness(
        const std::vector<std::vector<int>>& adj);
    
    /// \brief מציאת k-core של הגרף
    static std::vector<int> extract_k_core(
        const std::vector<std::vector<int>>& adj, int k);
};

} // namespace core
} // namespace lazymc

#include <gtest/gtest.h>
#include "lazymc/core/k_core.hpp"

namespace {

using namespace lazymc::core;

TEST(KCoreTest, ComputeCoreness) {
    std::vector<std::vector<int>> adj = {
        {1, 2},
        {0, 2, 3},
        {0, 1, 3},
        {1, 2}
    };
    
    auto coreness = KCore::compute_coreness(adj);
    
    ASSERT_EQ(coreness.size(), 4);
    // TODO: Add expected values when implemented
}

} // namespace

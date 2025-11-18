#include <gtest/gtest.h>
#include "lazymc/core/intersect.hpp"

namespace {

using namespace lazymc::core;

TEST(IntersectTest, SortedIntersection) {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {3, 4, 5, 6, 7};
    
    auto result = Intersect::sorted_intersection(a, b);
    
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], 3);
    EXPECT_EQ(result[1], 4);
    EXPECT_EQ(result[2], 5);
}

TEST(IntersectTest, EmptyIntersection) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};
    
    auto result = Intersect::sorted_intersection(a, b);
    
    EXPECT_TRUE(result.empty());
}

} // namespace

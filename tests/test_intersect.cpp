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

TEST(IntersectTest, HashIntersectionBasic) {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {3, 5, 7, 4};
    
    auto result = Intersect::hash_intersection(a, b);
    
    ASSERT_EQ(result.size(), 3);
    // Result should contain 3, 4, 5 (order from a)
    EXPECT_TRUE(std::find(result.begin(), result.end(), 3) != result.end());
    EXPECT_TRUE(std::find(result.begin(), result.end(), 4) != result.end());
    EXPECT_TRUE(std::find(result.begin(), result.end(), 5) != result.end());
}

TEST(IntersectTest, HashIntersectionNoCommon) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};
    
    auto result = Intersect::hash_intersection(a, b);
    
    EXPECT_TRUE(result.empty());
}

TEST(IntersectTest, HashIntersectionEmptyA) {
    std::vector<int> a;
    std::vector<int> b = {1, 2, 3};
    
    auto result = Intersect::hash_intersection(a, b);
    
    EXPECT_TRUE(result.empty());
}

TEST(IntersectTest, HashIntersectionEmptyB) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b;
    
    auto result = Intersect::hash_intersection(a, b);
    
    EXPECT_TRUE(result.empty());
}

TEST(IntersectTest, HashIntersectionDuplicates) {
    std::vector<int> a = {1, 2, 3, 2, 4};
    std::vector<int> b = {2, 3, 5};
    
    auto result = Intersect::hash_intersection(a, b);
    
    // Should have 2 (twice), 3
    ASSERT_EQ(result.size(), 3);
}

TEST(IntersectTest, HashIntersectionAllMatch) {
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {3, 1, 2};
    
    auto result = Intersect::hash_intersection(a, b);
    
    ASSERT_EQ(result.size(), 3);
}

} // namespace

// tests/intersect_functions_tests.cpp
#include <cstdint>
#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"
#include "lazymc/intersect_functions.h"

namespace {

using lazymc::intersect_gt;
using lazymc::intersect_size_gt_bool;

// -------- intersect_gt tests --------

TEST(IntersectGtTest, ReturnsMinusOneWhenAIsTooShortOrBIsTooSmall) {
    // n < theta
    std::vector<int32_t> A = {1, 2};
    std::unordered_set<int32_t> B = {1, 2, 3};
    std::vector<int32_t> C = {42, 43};  // should stay untouched in this branch

    int32_t theta = 3;
    int32_t res = intersect_gt(A, B, C, theta);

    EXPECT_EQ(-1, res);
    // C is not resized in this branch
    EXPECT_EQ(2u, C.size());
    EXPECT_EQ(42, C[0]);
    EXPECT_EQ(43, C[1]);

    // m < theta
    A = {1, 2, 3};
    B = {1};   // m = 1
    C.assign(3, 0);
    theta = 2;

    res = intersect_gt(A, B, C, theta);
    EXPECT_EQ(-1, res);
}

TEST(IntersectGtTest, IntersectionLargerThanThetaReturnsSizeAndWritesC) {
    // A ∩ B = {2, 4}, |∩| = 2, theta = 1  -> return 2
    std::vector<int32_t> A = {1, 2, 3, 4};
    std::unordered_set<int32_t> B = {2, 4, 6};
    std::vector<int32_t> C;

    int32_t theta = 1;
    int32_t res = intersect_gt(A, B, C, theta);

    EXPECT_EQ(2, res);                      // intersection size
    ASSERT_GE(C.size(), 2u);                // C is resized to |A|
    EXPECT_EQ(2, C[0]);                     // first intersection element
    EXPECT_EQ(4, C[1]);                     // second intersection element
}

TEST(IntersectGtTest, IntersectionSizeEqualToThetaOrSmallerReturnsMinusOne) {
    // |A ∩ B| = 2, theta = 2 -> -1
    std::vector<int32_t> A = {1, 2, 3, 4};
    std::unordered_set<int32_t> B = {2, 4, 6};
    std::vector<int32_t> C;

    int32_t theta = 2;
    int32_t res = intersect_gt(A, B, C, theta);
    EXPECT_EQ(-1, res);

    // |A ∩ B| = 0, theta = 1 -> -1
    A = {1, 3, 5};
    B = {2, 4, 6};
    C.clear();
    theta = 1;

    res = intersect_gt(A, B, C, theta);
    EXPECT_EQ(-1, res);
}

TEST(IntersectGtTest, ThetaZeroActsAsNonEmptyIntersection) {
    // theta = 0 -> returns |A ∩ B|, or 0 if there is no intersection
    std::vector<int32_t> A = {1, 2, 3, 4};
    std::unordered_set<int32_t> B = {2, 4};
    std::vector<int32_t> C;

    int32_t theta = 0;
    int32_t res = intersect_gt(A, B, C, theta);

    EXPECT_EQ(2, res);
    ASSERT_GE(C.size(), 2u);
    EXPECT_EQ(2, C[0]);
    EXPECT_EQ(4, C[1]);

    // no intersection
    A = {1, 3, 5};
    B = {2, 4, 6};
    C.clear();
    res = intersect_gt(A, B, C, theta);
    EXPECT_EQ(-1, res);
}

// Optional: behavior with duplicates in A (defined even if you do not use duplicates in practice)
TEST(IntersectGtTest, HandlesDuplicatesInAConsistently) {
    // A contains duplicates, B is a set. Each position in A is treated independently.
    std::vector<int32_t> A = {1, 2, 2, 3};
    std::unordered_set<int32_t> B = {2};
    std::vector<int32_t> C;

    int32_t theta = 0;
    int32_t res = intersect_gt(A, B, C, theta);

    // Two hits (both "2"s)
    EXPECT_EQ(2, res);
}

// -------- intersect_size_gt_bool tests --------

TEST(IntersectSizeGtBoolTest, ReturnsFalseWhenAOrBTooSmall) {
    std::vector<int32_t> A = {1, 2};
    std::unordered_set<int32_t> B = {1, 2};
    int32_t theta = 2;

    // n <= theta
    EXPECT_FALSE(intersect_size_gt_bool(A, B, theta));

    // m <= theta
    A = {1, 2, 3};
    B = {1, 2};
    EXPECT_FALSE(intersect_size_gt_bool(A, B, theta));
}

TEST(IntersectSizeGtBoolTest, CorrectWhenIntersectionIsLargerEqualOrSmaller) {
    std::vector<int32_t> A = {1, 2, 3, 4};
    std::unordered_set<int32_t> B = {2, 4, 6};

    // |∩| = 2, theta = 1 -> true
    EXPECT_TRUE(intersect_size_gt_bool(A, B, /*theta=*/1));

    // |∩| = 2, theta = 2 -> false
    EXPECT_FALSE(intersect_size_gt_bool(A, B, /*theta=*/2));

    // |∩| = 2, theta = 3 -> false
    EXPECT_FALSE(intersect_size_gt_bool(A, B, /*theta=*/3));
}

TEST(IntersectSizeGtBoolTest, ThetaZeroChecksNonEmptyIntersection) {
    std::vector<int32_t> A = {1, 2, 3, 4};
    std::unordered_set<int32_t> B = {2, 4};
    int32_t theta = 0;

    // non-empty intersection
    EXPECT_TRUE(intersect_size_gt_bool(A, B, theta));

    // empty intersection
    A = {1, 3, 5};
    B = {2, 4, 6};
    EXPECT_FALSE(intersect_size_gt_bool(A, B, theta));
}

TEST(IntersectSizeGtBoolTest, HandlesEmptyInputs) {
    std::vector<int32_t> A;
    std::unordered_set<int32_t> B;

    // Any theta >= 0 should be false on empty sets
    EXPECT_FALSE(intersect_size_gt_bool(A, B, /*theta=*/0));
    EXPECT_FALSE(intersect_size_gt_bool(A, B, /*theta=*/1));

    A = {1, 2, 3};
    B.clear();
    EXPECT_FALSE(intersect_size_gt_bool(A, B, /*theta=*/0));

    A.clear();
    B = {1, 2, 3};
    EXPECT_FALSE(intersect_size_gt_bool(A, B, /*theta=*/0));
}

}  // namespace

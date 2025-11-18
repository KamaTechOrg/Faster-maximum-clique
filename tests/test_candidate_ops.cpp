#include <gtest/gtest.h>
#include "lazymc/core/candidate_ops.hpp"

namespace {

using namespace lazymc::core;

TEST(CandidateOpsTest, ExtendClique) {
    CandidateOps::Candidate clique = {0, 1, 2};
    auto extended = CandidateOps::extend(clique, 3);
    
    ASSERT_EQ(extended.size(), 4);
    EXPECT_EQ(extended[3], 3);
}

// TODO: Add more tests when is_clique is implemented

} // namespace

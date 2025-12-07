#pragma once
#include <vector>
#include <cstdint>
#include <unordered_set>
#include "lazymc/bk.h"
namespace lazymc {


// Algorithm 3
int32_t intersect_gt(const std::vector<int32_t>& A,
                     const std::unordered_set<int32_t>& B,
                     std::vector<int32_t>& C,
                     int32_t theta);

// Algorithm 4
bool intersect_size_gt_bool(const std::vector<int32_t>& A,
                            const std::unordered_set<int32_t>& B,
                            int32_t theta);

} // namespace lazymc

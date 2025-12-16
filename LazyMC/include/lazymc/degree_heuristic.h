#pragma once

#include <cstdint>
#include <vector>

#include "lazymc/lazygraph.h"

namespace lazymc {

std::vector<int32_t> degree_based_heuristic(LazyGraph& H, int top_k);

} // namespace lazymc

#pragma once

#include <vector>
#include <cstdint>

#include "lazymc/lazygraph.h"

namespace lazymc {

// If AdjList is not defined in graph_types.h, you can temporarily add:
// using AdjList = std::vector<std::vector<int32_t>>;

// High-level LazyMC entry point (Algorithm 1).
// For now it only wires the main building blocks and returns a clique in G-labels.
std::vector<int32_t> LazyMC_run(const AdjList& G);

} // namespace lazymc

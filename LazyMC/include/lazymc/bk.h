#pragma once

#include <vector>
#include <map>
#include <unordered_map>


namespace lazymc {

/// Adjacency map type:
/// - key: vertex id (int)
/// - value: list of neighbors (each neighbor is an int as well)

using AdjMap = std::unordered_map<int, std::vector<int>>;


/// Bron–Kerbosch algorithm: enumerate all maximal cliques.
/// - Input: undirected simple graph represented as an adjacency map.
/// - Output: list of maximal cliques, each clique is a vector of vertex ids.
std::vector<std::vector<int>>
bron_kerbosch_all(const AdjMap& G);

/// Bron–Kerbosch-based search for a single maximum clique.
/// - Input: same adjacency map as above.
/// - Output: one maximum clique (largest size found).
///   If the graph is empty, returns an empty vector.
std::vector<int>
bron_kerbosch_maximum(const AdjMap& G);

} // namespace lazymc

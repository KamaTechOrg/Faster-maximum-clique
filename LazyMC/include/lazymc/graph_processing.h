#pragma once
#include <map>
#include <utility>
#include <vector>
// Builds an undirected adjacency list from the given edges.
std::map<int, std::vector<int>>
graph_processing(const std::vector<std::pair<int,int>>& edges);


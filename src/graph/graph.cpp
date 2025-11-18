// graph.cpp
#include "lazymc/graph/graph.hpp"

namespace lazymc {
namespace graph {

Graph::Graph(int num_vertices) : adj_(num_vertices) {}

void Graph::add_edge(VertexId u, VertexId v) {
    if (u >= num_vertices() || v >= num_vertices()) {
        return; // או לזרוק exception
    }
    adj_[u].push_back(v);
    adj_[v].push_back(u);
}

} // namespace graph
} // namespace lazymc

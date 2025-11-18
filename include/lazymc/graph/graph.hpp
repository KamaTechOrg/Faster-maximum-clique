// graph.hpp
#pragma once

#include <vector>
#include <cstdint>

namespace lazymc {
namespace graph {

/// \brief מייצג גרף בלתי-מכוון באמצעות רשימת שכנויות
class Graph {
public:
    using VertexId = int;
    using AdjList = std::vector<std::vector<VertexId>>;

    Graph() = default;
    explicit Graph(int num_vertices);
    
    /// \brief הוספת קשת בין שני קודקודים
    void add_edge(VertexId u, VertexId v);
    
    /// \brief החזרת מספר הקודקודים
    int num_vertices() const { return static_cast<int>(adj_.size()); }
    
    /// \brief החזרת רשימת השכנים של קודקוד
    const std::vector<VertexId>& neighbors(VertexId v) const { return adj_[v]; }
    
    /// \brief החזרת דרגת קודקוד
    int degree(VertexId v) const { return static_cast<int>(adj_[v].size()); }
    
    /// \brief גישה לרשימת השכנויות המלאה
    const AdjList& adjacency_list() const { return adj_; }

private:
    AdjList adj_;
};

} // namespace graph
} // namespace lazymc

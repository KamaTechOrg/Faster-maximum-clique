#include <iostream>
#include "lazymc/k_vertex_cover.h"

int main() {
    using lazymc::AdjList;
    using lazymc::k_vertex_cover_adj;
    using lazymc::k_vertex_cover_res;

    // Test 1: graph with 1 vertex and no edges, k = 0
    {
        AdjList G1(1); // one vertex: 0, no neighbors
        int k = 0;

        k_vertex_cover_res res = k_vertex_cover_adj(G1, k);
        std::cout << "Test 1 (single vertex, no edges, k=0): found = "
                  << res.found << ", cover size = " << res.cover.size() << "\n";
    }

    // Test 2: graph 0--1, k = 1
    {
        AdjList G2(2);
        G2[0].push_back(1);
        G2[1].push_back(0);

        int k = 1;
        k_vertex_cover_res res = k_vertex_cover_adj(G2, k);
        std::cout << "Test 2 (edge 0-1, k=1): found = "
                  << res.found << ", cover size = " << res.cover.size() << "\n";

        if (res.found) {
            std::cout << "  cover vertices:";
            for (int v : res.cover) {
                std::cout << " " << v;
            }
            std::cout << "\n";
        }
    }

    // Test 3: graph 0--1, k = 0
    {
        AdjList G3(2);
        G3[0].push_back(1);
        G3[1].push_back(0);

        int k = 0;
        k_vertex_cover_res res = k_vertex_cover_adj(G3, k);
        std::cout << "Test 3 (edge 0-1, k=0): found = "
                  << res.found << ", cover size = " << res.cover.size() << "\n";
    }

    return 0;
}

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <string>

#include "lazymc/k_vertex_cover.h"

// כאן אנחנו מניחים שבהדר מוגדר:
// namespace lazymc { using AdjList = std::vector<std::vector<int32_t>>; }

// ------------------------------------------------------------
// קריאת גרף מקובץ בפורמט:
//
//   n m
//   u1 v1
//   u2 v2
//   ...
//   um vm
//
// כרגע נניח שהקודקודים הם 0-based (0..n-1). אם אצלך 1-based אפשר
// להוסיף --u, --v כמו שעשית במקומות אחרים.
// ------------------------------------------------------------
lazymc::AdjList load_graph_with_header(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    int n = 0, m = 0;
    if (!(in >> n >> m)) {
        throw std::runtime_error("Failed to read n and m from file: " + path);
    }

    lazymc::AdjList G(n);

    for (int i = 0; i < m; ++i) {
        int u, v;
        if (!(in >> u >> v)) {
            throw std::runtime_error("Failed to read edge " + std::to_string(i));
        }

        // אם אצלך הקובץ 1-based, פשוט תבטל הערה על השורות הבאות:
        // --u;
        // --v;

        if (u == v) {
            continue; // לא צריך לולאות עצמיות
        }
        if (u < 0 || u >= n || v < 0 || v >= n) {
            throw std::runtime_error("Vertex id out of range in file: " + path);
        }

        G[u].push_back(v);
        G[v].push_back(u); // גרף בלתי-מכוון
    }

    // מיון וסילוק כפילויות
    for (auto& nbrs : G) {
        std::sort(nbrs.begin(), nbrs.end());
        nbrs.erase(std::unique(nbrs.begin(), nbrs.end()), nbrs.end());
    }

    return G;
}

int main(int argc, char** argv) {
    try {
        if (argc < 3) {
            std::cerr << "Usage: kvc_demo <graph_file> <k>\n";
            return 1;
        }

        std::string path = argv[1];
        int k = std::stoi(argv[2]);

        std::cout << "Loading graph from: " << path << "\n";
        std::cout << "k = " << k << "\n";

        lazymc::AdjList G = load_graph_with_header(path);

        // כאן אנחנו משתמשים בפונקציה שלך:
        // k_vertex_cover_res k_vertex_cover_adj(const AdjList& G, int32_t k)
        lazymc::k_vertex_cover_res res = lazymc::k_vertex_cover_adj(G, k);

        std::cout << "aborted = " << (res.aborted ? "true" : "false") << "\n";
        std::cout << "found   = " << (res.found   ? "true" : "false") << "\n";

        if (res.found) {
            std::cout << "cover size found = " << res.cover.size() << "\n";
            std::cout << "k                = " << k << "\n";
        }

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}

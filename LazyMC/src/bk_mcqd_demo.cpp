#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <string>
#include <limits>

#include "mcqd.h"
#include "lazymc/bk.h"

// ב-bk.h יש לך משהו כזה:
///  namespace lazymc { using AdjMap = std::map<int, std::vector<int>>; }

// ------------------------------------------------------------------
// קריאת גרף מקובץ בפורמט של רשימת קשתות:
//
//   u1 v1
//   u2 v2
//   ...
//   uk vk
//
// בלי שורה ראשונה של n m.
// ------------------------------------------------------------------
lazymc::AdjMap load_graph_from_edge_list(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::vector<std::pair<int,int>> edges;
    edges.reserve(1024);

    int u, v;
    int min_id = std::numeric_limits<int>::max();

    while (in >> u >> v) {
        edges.emplace_back(u, v);
        min_id = std::min(min_id, std::min(u, v));
    }

    if (edges.empty()) {
        return lazymc::AdjMap{};
    }

    // אם הקובץ שלך 1-based (קודקודים מתחילים מ-1) אפשר להמיר פה ל-0-based.
    // כרגע אני מניח שהוא כבר 0-based. אם תרצה – נעשה גרסה ל-1-based.
    bool assume_zero_based = true;
    (void)min_id; // שלא יציק קומפיילר על unused

    lazymc::AdjMap G;
    for (auto [a, b] : edges) {
        if (!assume_zero_based) {
            --a;
            --b;
        }
        if (a == b) continue; // בלי לולאות עצמיות

        G[a].push_back(b);
        G[b].push_back(a); // גרף בלתי-מכוון
    }

    // מיון וסילוק כפילויות
    for (auto& [vert, nbrs] : G) {
        std::sort(nbrs.begin(), nbrs.end());
        nbrs.erase(std::unique(nbrs.begin(), nbrs.end()), nbrs.end());
    }

    return G;
}

// ------------------------------------------------------------------
// בניית מטריצת שכנויות bool** מ-AdjMap בשביל MCQD
// ------------------------------------------------------------------
struct McqdMatrix {
    bool** conn = nullptr;
    int n = 0;
};

McqdMatrix build_mcqd_matrix_from_adjmap(const lazymc::AdjMap& G) {
    McqdMatrix out;

    if (G.empty()) {
        out.conn = nullptr;
        out.n = 0;
        return out;
    }

    // אוספים את כל הקודקודים (יכולים להיות לא רציפים)
    std::vector<int> ids;
    ids.reserve(G.size());
    for (const auto& [v, _] : G) {
        ids.push_back(v);
    }
    std::sort(ids.begin(), ids.end());

    const int n = static_cast<int>(ids.size());
    out.n = n;

    // מיפוי מזהה → אינדקס 0..n-1
    std::map<int, int> id_to_idx;
    for (int i = 0; i < n; ++i) {
        id_to_idx[ids[i]] = i;
    }

    // הקצאת מטריצה n×n
    bool** conn = new bool*[n];
    for (int i = 0; i < n; ++i) {
        conn[i] = new bool[n];
        std::fill(conn[i], conn[i] + n, false);
    }

    // מילוי שכנויות
    for (const auto& [u, nbrs] : G) {
        int iu = id_to_idx[u];
        for (int v : nbrs) {
            auto it = id_to_idx.find(v);
            if (it == id_to_idx.end()) continue;
            int iv = it->second;
            conn[iu][iv] = true;
            conn[iv][iu] = true;
        }
    }

    out.conn = conn;
    return out;
}

void free_mcqd_matrix(McqdMatrix& M) {
    if (!M.conn) return;
    for (int i = 0; i < M.n; ++i) {
        delete[] M.conn[i];
    }
    delete[] M.conn;
    M.conn = nullptr;
    M.n = 0;
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: bk_mcqd_demo <edge_list_file>\n";
            return 1;
        }

        std::string path = argv[1];
        std::cout << "Loading edge list from: " << path << "\n";

        // 1) טוענים את הגרף מרשימת קשתות – זה מה ש-BK שלך מצפה
        lazymc::AdjMap G = load_graph_from_edge_list(path);
        if (G.empty()) {
            std::cout << "Graph is empty.\n";
            return 0;
        }

        // 2) בונים מטריצה עבור MCQD
        McqdMatrix M = build_mcqd_matrix_from_adjmap(G);

        // 3) מריצים MCQD
        Maxclique mcq(reinterpret_cast<const bool* const*>(M.conn), M.n);
        int* qmax = new int[M.n];
        int qsize = 0;
        mcq.mcqdyn(qmax, qsize);
        int mcqd_size = qsize;

        // 4) מריצים Bron–Kerbosch שלך
        std::vector<int> bk_clique = lazymc::bron_kerbosch_maximum(G);
        int bk_size = static_cast<int>(bk_clique.size());

        // 5) מדפיסים ומשווים
        std::cout << "MCQD max clique size: " << mcqd_size << "\n";
        std::cout << "BK   max clique size: " << bk_size   << "\n";

        if (mcqd_size == bk_size) {
            std::cout << "OK: sizes match\n";
        } else {
            std::cout << "MISMATCH!\n";
        }

        delete[] qmax;
        free_mcqd_matrix(M);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}

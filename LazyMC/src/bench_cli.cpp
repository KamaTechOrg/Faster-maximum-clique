#include "lazymc/lazymc.h"
#include "lazymc/bk.h"
#include "lazymc/mcqd_adapter.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using AdjList = std::vector<std::vector<int32_t>>;
using AdjMap  = lazymc::AdjMap;

namespace {

static void die(const std::string& msg) {
    std::cerr << "Error: " << msg << "\n";
    std::exit(1);
}

static void sort_and_unique(AdjList& G) {
    for (auto& nbrs : G) {
        std::sort(nbrs.begin(), nbrs.end());
        nbrs.erase(std::unique(nbrs.begin(), nbrs.end()), nbrs.end());
    }
}

static bool has_edge_sorted(const AdjList& G, int32_t u, int32_t v) {
    const auto& nbrs = G[static_cast<std::size_t>(u)];
    return std::binary_search(nbrs.begin(), nbrs.end(), v);
}

static bool is_clique(const AdjList& G, const std::vector<int32_t>& C) {
    for (std::size_t i = 0; i < C.size(); ++i) {
        int32_t a = C[i];
        if (a < 0 || static_cast<std::size_t>(a) >= G.size()) return false;
        for (std::size_t j = i + 1; j < C.size(); ++j) {
            int32_t b = C[j];
            if (b < 0 || static_cast<std::size_t>(b) >= G.size()) return false;
            if (!has_edge_sorted(G, a, b) || !has_edge_sorted(G, b, a)) return false;
        }
    }
    return true;
}

static std::int64_t count_edges_undirected(const AdjList& G) {
    std::int64_t m2 = 0;
    for (const auto& nbrs : G) m2 += static_cast<std::int64_t>(nbrs.size());
    return m2 / 2;
}

static AdjMap to_adjmap(const AdjList& G) {
    AdjMap M;
    for (std::size_t v = 0; v < G.size(); ++v) {
        M[static_cast<int32_t>(v)] = G[v];
    }
    return M;
}

// Supported formats:
// (1) Edge list:
//     n m
//     u v
//     ...
// (2) DIMACS clique:
//     p edge n m
//     e u v
//     ...
static AdjList read_graph(std::istream& in, bool one_based) {
    std::string line;

    auto next_line = [&](std::string& out) -> bool {
        while (std::getline(in, out)) {
            if (out.empty()) continue;
            if (out[0] == '#') continue;
            if (out[0] == 'c') continue; // DIMACS comment
            return true;
        }
        return false;
    };

    if (!next_line(line)) die("Empty input.");

    // DIMACS
    if (!line.empty() && line[0] == 'p') {
        std::istringstream iss(line);
        std::string p, kind;
        int n = 0, m = 0;
        iss >> p >> kind >> n >> m;
        if (!iss || n <= 0) die("Bad DIMACS header: " + line);

        AdjList G(static_cast<std::size_t>(n));

        while (next_line(line)) {
            if (!line.empty() && line[0] == 'e') {
                std::istringstream eiss(line);
                char e;
                int u, v;
                eiss >> e >> u >> v;
                if (!eiss) die("Bad DIMACS edge: " + line);
                if (one_based) { --u; --v; }
                if (u < 0 || v < 0 || u >= n || v >= n) die("Vertex out of range: " + line);
                if (u == v) continue;
                G[static_cast<std::size_t>(u)].push_back(v);
                G[static_cast<std::size_t>(v)].push_back(u);
            }
        }

        sort_and_unique(G);
        return G;
    }

    // Edge list "n m"
    int n = 0;
    long long m = 0;
    {
        std::istringstream iss(line);
        iss >> n >> m;
        if (!iss || n <= 0) die("Expected 'n m' or DIMACS header. Got: " + line);
    }

    AdjList G(static_cast<std::size_t>(n));
    long long read = 0;

    while (read < m && next_line(line)) {
        std::istringstream eiss(line);
        int u, v;
        eiss >> u >> v;
        if (!eiss) die("Bad edge line: " + line);
        if (one_based) { --u; --v; }
        if (u < 0 || v < 0 || u >= n || v >= n) die("Vertex out of range: " + line);
        if (u == v) continue;
        G[static_cast<std::size_t>(u)].push_back(v);
        G[static_cast<std::size_t>(v)].push_back(u);
        ++read;
    }

    sort_and_unique(G);
    return G;
}

enum class Solver { LazyMC, MCQD, BK, All };

static Solver parse_solver(const std::string& s) {
    if (s == "lazymc") return Solver::LazyMC;
    if (s == "mcqd")   return Solver::MCQD;
    if (s == "bk")     return Solver::BK;
    if (s == "all")    return Solver::All;
    die("Bad --solver value: " + s + " (expected lazymc|mcqd|bk|all)");
    return Solver::All;
}

template <class F>
static double time_ms(F&& f) {
    const auto t1 = std::chrono::steady_clock::now();
    f();
    const auto t2 = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(t2 - t1).count();
}

} // namespace

int main(int argc, char** argv) {
    std::string input_path;
    bool one_based = false;
    bool print_clique = false;
    Solver solver = Solver::All;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--input" && i + 1 < argc) input_path = argv[++i];
        else if (a == "--one-based") one_based = true;
        else if (a == "--print-clique") print_clique = true;
        else if (a == "--solver" && i + 1 < argc) solver = parse_solver(argv[++i]);
        else if (a == "-h" || a == "--help") {
            std::cout
                << "Usage:\n"
                << "  bench_cli --input <file> [--one-based] [--print-clique] [--solver <name>]\n"
                << "\n"
                << "Solvers:\n"
                << "  --solver all     (default)\n"
                << "  --solver lazymc\n"
                << "  --solver mcqd\n"
                << "  --solver bk\n"
                << "\nFormats:\n"
                << "  Edge list: n m \\n u v \\n ...\n"
                << "  DIMACS:    p edge n m \\n e u v \\n ...\n";
            return 0;
        } else {
            die("Unknown arg: " + a);
        }
    }

    if (input_path.empty()) die("Missing --input <file>.");

    std::ifstream fin(input_path);
    if (!fin) die("Cannot open: " + input_path);

    AdjList G = read_graph(fin, one_based);
    const std::int64_t m = count_edges_undirected(G);

    std::cout << "Graph: n=" << G.size() << " m=" << m << "\n";

    // ---- LazyMC ----
    if (solver == Solver::LazyMC || solver == Solver::All) {
        std::vector<int32_t> C;
        const double ms = time_ms([&]() { C = lazymc::LazyMC_run(G); });

        const bool ok = is_clique(G, C);
        std::cout << "[LazyMC] clique_size=" << C.size()
                  << " time_ms=" << ms
                  << " valid=" << (ok ? "yes" : "NO") << "\n";

        if (print_clique) {
            std::cout << "[LazyMC] clique:";
            for (int32_t v : C) std::cout << " " << v;
            std::cout << "\n";
        }
    }

    // ---- MCQD ----
    if (solver == Solver::MCQD || solver == Solver::All) {
        lazymc::McqdResult mcqd_res;
        const double ms = time_ms([&]() { mcqd_res = lazymc::mcqd_max_clique(G, 0.025); });

        const bool ok = is_clique(G, mcqd_res.clique);
        std::cout << "[MCQD]  clique_size=" << mcqd_res.clique.size()
                  << " time_ms=" << ms
                  << " valid=" << (ok ? "yes" : "NO")
                  << " (exact baseline)\n";

        if (print_clique) {
            std::cout << "[MCQD] clique:";
            for (int32_t v : mcqd_res.clique) std::cout << " " << v;
            std::cout << "\n";
        }
    }

    // ---- BK ----
    if (solver == Solver::BK || solver == Solver::All) {
        AdjMap M = to_adjmap(G);

        std::vector<int32_t> Cb;
        const double ms = time_ms([&]() { Cb = lazymc::bron_kerbosch_maximum(M); });

        const bool ok = is_clique(G, Cb);
        std::cout << "[BK]    clique_size=" << Cb.size()
                  << " time_ms=" << ms
                  << " valid=" << (ok ? "yes" : "NO") << "\n";

        if (print_clique) {
            std::cout << "[BK] clique:";
            for (int32_t v : Cb) std::cout << " " << v;
            std::cout << "\n";
        }
    }

    return 0;
}

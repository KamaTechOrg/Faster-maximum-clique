// Golden model of the FPGA clique engine (rtl/clique_engine.v).
//
// Algorithm (same as the RTL, step for step):
//   node(P, L):  greedy colouring of P  -> k colours, last coloured vertex v
//                if L + k <= best: return to parent continuation
//                push continuation P \ {v}; R[L] = v; L++
//                NP = P & N(v); if NP == 0: record L if better, pop
//                else node(NP, L)
//   (binary branching on the last vertex of the highest colour class; each
//    stack level stores one N-bit row, so depth*N bits of on-chip memory.)
//
// Cycle accounting mirrors the RTL state machine exactly, so the simulated
// cycle count can be checked against it.
//
// Input: "S n init_best" header followed by "u v" edge lines (0-based),
//        or DIMACS "p edge n m" / "e u v" (1-based).
// Usage: mc_model <graph> [--emit-hex <prefix>]
//   --emit-hex writes <prefix>.adj.hex (one N-bit row per line, LSB = vertex 0)
//   and <prefix>.cfg (n init_best expected_best expected_cycles) for the testbench.
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using W = std::uint64_t;

struct Graph {
    int n = 0, nw = 0, init_best = 0;
    std::vector<W> adj;
    W* row(int v) { return &adj[static_cast<std::size_t>(v) * nw]; }
    const W* row(int v) const { return &adj[static_cast<std::size_t>(v) * nw]; }
    void add(int a, int b) {
        if (a == b) return;
        row(a)[b >> 6] |= W{1} << (b & 63);
        row(b)[a >> 6] |= W{1} << (a & 63);
    }
};

static Graph load(const std::string& path) {
    std::ifstream f(path);
    if (!f) { std::cerr << "cannot open " << path << "\n"; std::exit(1); }
    Graph g;
    std::string line;
    bool dimacs = false;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == 'c' || line[0] == '#') continue;
        std::istringstream is(line);
        if (line[0] == 'S') {
            std::string s; is >> s >> g.n >> g.init_best;
            g.nw = (g.n + 63) / 64; g.adj.assign(static_cast<std::size_t>(g.n) * g.nw, 0);
        } else if (line[0] == 'p') {
            std::string p, e; long long m; is >> p >> e >> g.n >> m; dimacs = true;
            g.nw = (g.n + 63) / 64; g.adj.assign(static_cast<std::size_t>(g.n) * g.nw, 0);
        } else if (line[0] == 'e') {
            char c; int a, b; is >> c >> a >> b; g.add(a - 1, b - 1);
        } else {
            int a, b; is >> a >> b; g.add(dimacs ? a - 1 : a, dimacs ? b - 1 : b);
        }
    }
    return g;
}

struct Engine {
    const Graph& g;
    int nw;
    int best;
    std::vector<int> R, bestR;
    long long nodes = 0, cycles = 0;

    explicit Engine(const Graph& gr) : g(gr), nw(gr.nw), best(gr.init_best) {}

    static bool empty(const std::vector<W>& s) { for (W w : s) if (w) return false; return true; }

    // returns number of colours, sets last coloured vertex
    int colour(const std::vector<W>& P, int& last) {
        std::vector<W> U = P, Q(nw);
        int k = 0;
        cycles += 1;                                   // C_INIT
        for (;;) {
            cycles += 1;                               // C_CLASS
            if (empty(U)) return k;
            Q = U; ++k;
            for (;;) {
                cycles += 1;                           // C_PICK
                int i = 0; while (i < nw && !Q[i]) ++i;
                if (i == nw) break;
                const int v = i * 64 + __builtin_ctzll(Q[i]);
                last = v;
                Q[i] &= ~(W{1} << (v & 63)); U[i] &= ~(W{1} << (v & 63));
                cycles += 1;                           // C_UPD
                const W* r = g.row(v);
                for (int j = 0; j < nw; ++j) Q[j] &= ~r[j];
            }
        }
    }

    void run() {
        std::vector<std::vector<W>> stack;             // continuation P per level
        std::vector<W> P(nw, 0);
        for (int v = 0; v < g.n; ++v) P[v >> 6] |= W{1} << (v & 63);
        int L = 0;
        for (;;) {
            // NODE
            ++nodes;
            int v = -1;
            const int k = colour(P, v);
            cycles += 1;                               // C_DONE
            bool pop = (L + k <= best);
            if (!pop) {
                cycles += 1;                           // B_AND
                std::vector<W> NP(nw), cont = P;
                const W* r = g.row(v);
                for (int j = 0; j < nw; ++j) NP[j] = P[j] & r[j];
                cont[v >> 6] &= ~(W{1} << (v & 63));
                stack.push_back(cont);
                R.resize(static_cast<std::size_t>(L)); R.push_back(v);
                ++L;
                if (empty(NP)) {
                    if (L > best) { best = L; bestR = R; cycles += L; } // REC: one vertex per cycle
                    pop = true;
                } else {
                    P = NP;
                }
            }
            if (pop) {
                cycles += 1;                           // POP
                if (L == 0) break;
                --L;
                cycles += 1;                           // POP2
                P = stack.back(); stack.pop_back();
            }
        }
    }
};

static bool is_clique(const Graph& g, const std::vector<int>& c) {
    for (std::size_t i = 0; i < c.size(); ++i)
        for (std::size_t j = i + 1; j < c.size(); ++j)
            if (!((g.row(c[i])[c[j] >> 6] >> (c[j] & 63)) & 1)) return false;
    return true;
}

// Host-side relabelling (no hardware change): the colouring visits vertices in
// index order, so the order decides colour quality and search size.
//   none   : as given
//   deg    : non-increasing degree (MCQ)
//   degen  : degeneracy order, highest core first (reverse smallest-last, MCS-like)
static Graph relabel(const Graph& g, const std::string& mode) {
    if (mode == "none") return g;
    const int n = g.n;
    std::vector<int> deg(n), order;
    for (int v = 0; v < n; ++v) { int d = 0; for (int i = 0; i < g.nw; ++i) d += __builtin_popcountll(g.row(v)[i]); deg[v] = d; }
    if (mode == "deg") {
        order.resize(n); for (int v = 0; v < n; ++v) order[v] = v;
        std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return deg[a] > deg[b]; });
    } else { // degen: repeatedly remove min-degree vertex; reverse removal order
        std::vector<int> d = deg; std::vector<char> gone(n, 0);
        for (int it = 0; it < n; ++it) {
            int m = -1; for (int v = 0; v < n; ++v) if (!gone[v] && (m < 0 || d[v] < d[m])) m = v;
            gone[m] = 1; order.push_back(m);
            for (int u = 0; u < n; ++u) if (!gone[u] && ((g.row(m)[u >> 6] >> (u & 63)) & 1)) --d[u];
        }
        std::reverse(order.begin(), order.end());
    }
    std::vector<int> pos(n); for (int i = 0; i < n; ++i) pos[order[i]] = i;
    Graph h; h.n = n; h.nw = g.nw; h.init_best = g.init_best; h.adj.assign(g.adj.size(), 0);
    for (int a = 0; a < n; ++a) for (int b = a + 1; b < n; ++b)
        if ((g.row(a)[b >> 6] >> (b & 63)) & 1) h.add(pos[a], pos[b]);
    return h;
}

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "usage: mc_model <graph> [--order none|deg|degen] [--emit-hex prefix]\n"; return 1; }
    std::string order = "none";
    for (int i = 2; i + 1 < argc; ++i) if (std::string(argv[i]) == "--order") order = argv[i + 1];
    Graph g = relabel(load(argv[1]), order);
    Engine e(g);
    const auto t0 = std::chrono::steady_clock::now();
    e.run();
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    const bool ok = e.bestR.empty() || is_clique(g, e.bestR);
    std::printf("n=%d init_best=%d best=%d nodes=%lld cycles=%lld cpu_ms=%.3f valid=%s\n",
                g.n, g.init_best, e.best, e.nodes, e.cycles, ms, ok ? "yes" : "NO");

    std::string pre;
    for (int i = 2; i + 1 < argc; ++i) if (std::string(argv[i]) == "--emit-hex") pre = argv[i + 1];
    if (!pre.empty()) {
        std::ofstream h(pre + ".adj.hex");
        const int digits = (g.nw * 64) / 4;
        for (int v = 0; v < g.n; ++v) {
            std::string s(static_cast<std::size_t>(digits), '0');
            for (int b = 0; b < g.nw * 64; ++b)
                if ((g.row(v)[b >> 6] >> (b & 63)) & 1) {
                    const int d = digits - 1 - b / 4;
                    int val = (s[static_cast<std::size_t>(d)] <= '9') ? s[d] - '0' : s[d] - 'a' + 10;
                    val |= 1 << (b % 4);
                    s[static_cast<std::size_t>(d)] = "0123456789abcdef"[val];
                }
            h << s << "\n";
        }
        std::ofstream c(pre + ".cfg");
        c << g.n << " " << g.init_best << " " << e.best << " " << e.cycles << " " << e.nodes << "\n";
    }
    return ok ? 0 : 2;
}

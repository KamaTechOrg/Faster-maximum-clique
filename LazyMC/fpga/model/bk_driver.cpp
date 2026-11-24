// Times the repository's bitset BK (bron_kerbosch_maximum_bitset) on one dumped
// subproblem ("S n init_best" + "u v" lines), single thread, for comparison.
#include "lazymc/bk_bitset.h"
#include "lazymc/incumbent.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <numeric>
#include <sstream>
#include <string>
int main(int argc, char** argv) {
    std::ifstream f(argv[1]); std::string line, s; int n = 0, best = 0;
    lazymc::AdjList G;
    while (std::getline(f, line)) {
        std::istringstream is(line);
        if (line[0] == 'S') { is >> s >> n >> best; G.assign(n, {}); }
        else { int a, b; is >> a >> b; G[a].push_back(b); G[b].push_back(a); }
    }
    for (auto& r : G) std::sort(r.begin(), r.end());
    std::vector<int32_t> verts(n); std::iota(verts.begin(), verts.end(), 0);
    lazymc::reset_incumbent();
    auto t0 = std::chrono::steady_clock::now();
    auto c = lazymc::bron_kerbosch_maximum_bitset(G, verts, best);
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    std::printf("bk_bitset: n=%d init_best=%d found=%zu nodes=%lld cpu_ms=%.3f\n", n, best, c.size(),
                lazymc::g_bk_nodes.load(), ms);
}

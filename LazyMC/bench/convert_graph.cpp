// Converts a raw graph file (SNAP edge list or Network Repository .edges/.mtx) read
// from stdin into the formats used by the comparison:
//   <out>.el            "n m" then "u v" per undirected edge, 0-based (LazyMC bench_cli, dOmega -e)
//   <out>.brb/b_degree.bin, b_adj.bin   MC-BRB binary format
// Vertex ids are compacted to 0..n-1 in order of first appearance; self loops and
// duplicate / reverse-duplicate edges are removed.
// Usage: zcat file.gz | convert_graph <out> [--mtx]
//   --mtx : skip the first non-comment line (Matrix Market size header)
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 2) { std::fprintf(stderr, "usage: convert_graph <out> [--mtx]\n"); return 1; }
    const std::string out = argv[1];
    const bool mtx = argc > 2 && std::strcmp(argv[2], "--mtx") == 0;

    std::vector<int64_t> id_map;               // raw id -> compact id (or -1)
    int32_t n = 0;
    auto get_id = [&](int64_t raw) -> int32_t {
        if (raw < 0) { std::fprintf(stderr, "negative id\n"); std::exit(1); }
        if (static_cast<std::size_t>(raw) >= id_map.size()) id_map.resize(static_cast<std::size_t>(raw) * 2 + 16, -1);
        int64_t& m = id_map[static_cast<std::size_t>(raw)];
        if (m < 0) m = n++;
        return static_cast<int32_t>(m);
    };

    std::vector<uint64_t> edges;               // (min << 32) | max
    static char buf[1 << 16];
    bool header_skipped = !mtx;
    while (std::fgets(buf, sizeof buf, stdin)) {
        const char* p = buf;
        while (*p == ' ' || *p == '\t') ++p;
        if (*p == '#' || *p == '%' || *p == '\n' || *p == '\r' || *p == 0) continue;
        if (!header_skipped) { header_skipped = true; continue; }
        char* e;
        const long long a = std::strtoll(p, &e, 10);
        if (e == p) continue;
        p = e; while (*p == ' ' || *p == '\t' || *p == ',') ++p;
        const long long b = std::strtoll(p, &e, 10);
        if (e == p) continue;
        if (a == b) continue;
        const uint32_t u = static_cast<uint32_t>(get_id(a)), v = static_cast<uint32_t>(get_id(b));
        if (u == v) continue;
        edges.push_back(u < v ? (uint64_t(u) << 32 | v) : (uint64_t(v) << 32 | u));
    }
    std::sort(edges.begin(), edges.end());
    edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
    const uint64_t m = edges.size();

    // edge list
    {
        FILE* f = std::fopen((out + ".el").c_str(), "w");
        std::fprintf(f, "%d %llu\n", n, static_cast<unsigned long long>(m));
        std::vector<char> ob; ob.reserve(1 << 22);
        char tmp[32];
        for (uint64_t x : edges) {
            int len = std::snprintf(tmp, sizeof tmp, "%u %u\n", uint32_t(x >> 32), uint32_t(x));
            ob.insert(ob.end(), tmp, tmp + len);
            if (ob.size() > (1u << 22) - 64) { std::fwrite(ob.data(), 1, ob.size(), f); ob.clear(); }
        }
        std::fwrite(ob.data(), 1, ob.size(), f);
        std::fclose(f);
    }

    // MC-BRB binary (CSR, both directions)
    {
        const std::string dir = out + ".brb";
        mkdir(dir.c_str(), 0755);
        std::vector<uint32_t> deg(static_cast<std::size_t>(n), 0);
        for (uint64_t x : edges) { ++deg[x >> 32]; ++deg[uint32_t(x)]; }
        std::vector<uint64_t> start(static_cast<std::size_t>(n) + 1, 0);
        for (int32_t i = 0; i < n; ++i) start[i + 1] = start[i] + deg[i];
        std::vector<uint32_t> adj(2 * m);
        std::vector<uint64_t> pos(start.begin(), start.end() - 1);
        for (uint64_t x : edges) {
            const uint32_t a = uint32_t(x >> 32), b = uint32_t(x);
            adj[pos[a]++] = b; adj[pos[b]++] = a;
        }
        FILE* f = std::fopen((dir + "/b_degree.bin").c_str(), "wb");
        const uint32_t tt = sizeof(int), nn = static_cast<uint32_t>(n), mm = static_cast<uint32_t>(2 * m);
        std::fwrite(&tt, 4, 1, f); std::fwrite(&nn, 4, 1, f); std::fwrite(&mm, 4, 1, f);
        std::fwrite(deg.data(), 4, deg.size(), f);
        std::fclose(f);
        f = std::fopen((dir + "/b_adj.bin").c_str(), "wb");
        std::fwrite(adj.data(), 4, adj.size(), f);
        std::fclose(f);
    }
    std::printf("%s: n=%d m=%llu\n", out.c_str(), n, static_cast<unsigned long long>(m));
    return 0;
}

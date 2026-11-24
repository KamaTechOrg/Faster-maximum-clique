// Bitset variant of k-Vertex-Cover on the complement of G[verts].
//
// Same search as k_vertex_cover_adj() (same kernelization rules in the same
// order, same greedy-matching bound, same max-degree branching, same step
// limit), so it visits the same number of steps. Only the representation
// differs:
//   active set A, cover set   -> bit vectors (copying a child = nw words)
//   deg(v)                    -> popcount(C[v] & A)
//   N(v) ∩ A                  -> C[v] & A
// where C[v] is row v of the complement adjacency matrix.
#include "lazymc/k_vertex_cover_bitset.h"
#include "lazymc/bk_bitset.h"

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace lazymc {

namespace {

using Word = std::uint64_t;
constexpr long long VC_STEP_LIMIT = 20'000'000; // same as k_vertex_cover.cpp

struct Ctx {
    int n = 0, nw = 0;
    std::vector<Word> C;   // complement adjacency, n * nw
    long long steps = 0;
    bool aborted = false;
    const Word* row(int v) const { return &C[static_cast<std::size_t>(v) * nw]; }
};

struct State {
    std::vector<Word> A;      // active vertices
    std::vector<Word> cover;  // vertices taken into the cover
    std::vector<int32_t> d;   // degree in the active graph (kept up to date)
    int k = 0;
};

inline bool test(const std::vector<Word>& s, int v) { return (s[v >> 6] >> (v & 63)) & 1; }
inline void clr(std::vector<Word>& s, int v) { s[v >> 6] &= ~(Word{1} << (v & 63)); }
inline void set(std::vector<Word>& s, int v) { s[v >> 6] |= Word{1} << (v & 63); }

inline int deg(const Ctx&, const State& s, int v) { return s.d[static_cast<std::size_t>(v)]; }

// First active neighbour of v (lowest index), or -1.
inline int first_nbr(const Ctx& c, const State& s, int v, const std::vector<Word>* exclude = nullptr) {
    const Word* r = c.row(v);
    for (int i = 0; i < c.nw; ++i) {
        Word w = r[i] & s.A[i];
        if (exclude) w &= ~(*exclude)[i];
        if (w) return i * 64 + __builtin_ctzll(w);
    }
    return -1;
}

template <class F> inline void for_each(const std::vector<Word>& s, int nw, F f) {
    for (int i = 0; i < nw; ++i) {
        Word w = s[i];
        while (w) { f(i * 64 + __builtin_ctzll(w)); w &= w - 1; }
    }
}

// Mirrors take_vertex_into_cover(): returns false if k < 0.
inline bool take(const Ctx& c, State& s, int v) {
    if (!test(s.A, v)) return true;
    set(s.cover, v);
    if (--s.k < 0) return false;
    clr(s.A, v);
    s.d[static_cast<std::size_t>(v)] = 0;
    const Word* r = c.row(v);
    for (int i = 0; i < c.nw; ++i) {
        Word w = r[i] & s.A[i];
        while (w) { --s.d[static_cast<std::size_t>(i * 64 + __builtin_ctzll(w))]; w &= w - 1; }
    }
    return true;
}

bool kernelize(const Ctx& c, State& s) {
    while (true) {
        if (s.k < 0) return false;
        bool changed = false;

        // 1. remove isolated vertices
        for (int i = 0; i < c.nw; ++i) {
            Word w = s.A[i];
            while (w) {
                const int v = i * 64 + __builtin_ctzll(w);
                w &= w - 1;
                if (deg(c, s, v) == 0) { clr(s.A, v); changed = true; }
            }
        }
        if (changed) continue;

        // 2. degree-1 rule: first u (in index order) with deg 1 -> take its neighbour
        int u1 = -1;
        for_each(s.A, c.nw, [&](int u) { if (u1 < 0 && deg(c, s, u) == 1) u1 = u; });
        if (u1 >= 0) {
            const int v = first_nbr(c, s, u1);
            if (v >= 0) {
                if (!take(c, s, v)) return false;
                continue;
            }
        }

        // 3. high-degree rule: first v with deg > k must be in the cover
        int vh = -1;
        for_each(s.A, c.nw, [&](int v) { if (vh < 0 && deg(c, s, v) > s.k) vh = v; });
        if (vh >= 0) {
            if (!take(c, s, vh)) return false;
            continue;
        }
        break;
    }
    return s.k >= 0;
}

// Greedy matching lower bound, same visiting order as the original.
int matching_lb(const Ctx& c, const State& s, int k_for_prune) {
    std::vector<Word> used(c.nw, 0);
    int m = 0;
    for (int i = 0; i < c.nw; ++i) {
        for (int b = 0; b < 64; ++b) {
            const int v = i * 64 + b;
            if (v >= c.n) break;
            if (!test(s.A, v) || test(used, v)) continue;
            const int u = first_nbr(c, s, v, &used);
            if (u < 0) continue;
            set(used, v); set(used, u);
            if (++m > k_for_prune) return m;
        }
    }
    return m;
}

bool solve(Ctx& c, State& s) {
    if (c.steps > VC_STEP_LIMIT) { c.aborted = true; return false; }
    ++c.steps;

    if (!kernelize(c, s)) return false;
    if (c.steps > VC_STEP_LIMIT) { c.aborted = true; return false; }

    // pick max-degree active vertex (first one on ties); also tells us if edges remain
    int best_v = -1, best_d = -1;
    for_each(s.A, c.nw, [&](int v) {
        const int d = deg(c, s, v);
        if (d > best_d) { best_d = d; best_v = v; }
    });
    if (best_v < 0 || best_d <= 0) return true; // no edges left: cover is valid

    if (matching_lb(c, s, s.k) > s.k) return false;

    const int v = best_v;

    // Branch A: v into the cover
    {
        State child = s;
        if (take(c, child, v) && solve(c, child)) {
            if (c.aborted) return false;
            s.cover.swap(child.cover);
            return true;
        }
        if (c.aborted) return false;
    }
    // Branch B: all active neighbours of v into the cover
    {
        if (best_d <= s.k) {
            State child = s;
            bool ok = true;
            const Word* r = c.row(v);
            for (int i = 0; i < c.nw && ok; ++i) {
                Word w = r[i] & s.A[i];
                while (w && ok) {
                    ok = take(c, child, i * 64 + __builtin_ctzll(w));
                    w &= w - 1;
                }
            }
            if (ok && solve(c, child)) {
                if (c.aborted) return false;
                s.cover.swap(child.cover);
                return true;
            }
        }
        if (c.aborted) return false;
    }
    return false;
}

} // namespace

k_vertex_cover_res k_vertex_cover_induced_complement_bitset(const AdjList& G,
                                                            const std::vector<int32_t>& verts,
                                                            int32_t k) {
    k_vertex_cover_res res;
    if (k < 0) return res;

    Ctx c;
    c.n = static_cast<int>(verts.size());
    c.nw = (c.n + 63) / 64;
    c.C.assign(static_cast<std::size_t>(c.n) * c.nw, 0);

    // label -> local index (position in verts) without any G-sized array
    std::vector<std::pair<int32_t, int32_t>> idx(verts.size());
    for (int i = 0; i < c.n; ++i) idx[i] = {verts[i], i};
    std::sort(idx.begin(), idx.end());

    // start with the complete graph minus self loops, then remove edges of G
    for (int i = 0; i < c.n; ++i) {
        Word* r = &c.C[static_cast<std::size_t>(i) * c.nw];
        for (int j = 0; j < c.n; ++j) if (j != i) r[j >> 6] |= Word{1} << (j & 63);
        for (int32_t y : G[static_cast<std::size_t>(verts[i])]) {
            auto it = std::lower_bound(idx.begin(), idx.end(), std::make_pair(y, INT32_MIN));
            if (it != idx.end() && it->first == y) r[it->second >> 6] &= ~(Word{1} << (it->second & 63));
        }
    }

    State s;
    s.A.assign(c.nw, 0);
    s.cover.assign(c.nw, 0);
    s.k = k;
    for (int v = 0; v < c.n; ++v) set(s.A, v);
    s.d.resize(static_cast<std::size_t>(c.n));
    for (int v = 0; v < c.n; ++v) {
        int dv = 0;
        for (int i = 0; i < c.nw; ++i) dv += __builtin_popcountll(c.row(v)[i]);
        s.d[static_cast<std::size_t>(v)] = dv;
    }

    if (matching_lb(c, s, k) > k) return res; // proven: no cover of size <= k

    const bool ok = solve(c, s);
    g_kvc_steps.fetch_add(c.steps, std::memory_order_relaxed);
    if (c.aborted) g_kvc_aborts.fetch_add(1, std::memory_order_relaxed);

    res.aborted = c.aborted;
    if (!ok || c.aborted) return res;
    res.found = true;
    for_each(s.cover, c.nw, [&](int v) { res.cover.push_back(v); });
    return res;
}

} // namespace lazymc

// Bitset variant of bron_kerbosch_maximum (same algorithm: BK + pivot that
// maximises |N(u) ∩ P| over P ∪ X, plus greedy-colouring bound when |P| <= 100).
// Only the set representation differs: P, X and N(v) are bit vectors, so
// "P ∩ N(v)" is a word-wise AND and |.| is popcount.
#include "lazymc/bk_bitset.h"
#include "lazymc/incumbent.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <vector>

namespace lazymc {

std::atomic<long long> g_bk_nodes{0};
std::atomic<long long> g_bk_ns{0};

namespace {

using Word = std::uint64_t;

struct BitBK {
    int n = 0, nw = 0;
    std::vector<Word> adj;              // n * nw
    std::vector<int32_t> labels;        // local -> G label
    std::vector<int> R, best;
    std::size_t init_best = 0;
    long long nodes = 0;

    const Word* row(int v) const { return &adj[static_cast<std::size_t>(v) * nw]; }

    static int popcount(const Word* s, int nw) {
        int c = 0;
        for (int i = 0; i < nw; ++i) c += __builtin_popcountll(s[i]);
        return c;
    }
    int and_count(const Word* a, const Word* b) const {
        int c = 0;
        for (int i = 0; i < nw; ++i) c += __builtin_popcountll(a[i] & b[i]);
        return c;
    }

    // First-fit greedy colouring in increasing vertex order (== original order).
    int colour_bound(const Word* P) const {
        std::vector<Word> U(P, P + nw), Q(nw);
        int k = 0;
        for (;;) {
            bool any = false;
            for (int i = 0; i < nw; ++i) if (U[i]) { any = true; break; }
            if (!any) return k;
            ++k;
            Q = U;
            for (int i = 0; i < nw; ++i) {
                while (Q[i]) {
                    const int b = __builtin_ctzll(Q[i]);
                    const int v = i * 64 + b;
                    Q[i] &= Q[i] - 1;
                    U[i] &= ~(Word{1} << b);
                    const Word* r = row(v);
                    for (int j = i; j < nw; ++j) Q[j] &= ~r[j];
                }
            }
        }
    }

    void recurse(std::vector<Word>& P, std::vector<Word>& X) {
        ++nodes;
        const std::size_t global_best =
            static_cast<std::size_t>(g_best_size.load(std::memory_order_relaxed));
        const std::size_t current_best =
            std::max({best.size(), global_best, init_best});

        const int p_size = popcount(P.data(), nw);
        if (R.size() + static_cast<std::size_t>(p_size) <= current_best) return;
        if (p_size <= 100) {
            if (R.size() + static_cast<std::size_t>(colour_bound(P.data())) <= current_best) return;
        }
        if (p_size == 0) {
            if (popcount(X.data(), nw) == 0 && R.size() > std::max(best.size(), init_best)) {
                best = R;
                if (R.size() > global_best) {
                    std::vector<int32_t> c;
                    c.reserve(R.size());
                    for (int v : R) c.push_back(labels[static_cast<std::size_t>(v)]);
                    try_update_incumbent(std::move(c));
                }
            }
            return;
        }

        // pivot over P ∪ X
        int pivot = -1, max_deg = 0;
        for (const auto* S : {&P, &X}) {
            for (int i = 0; i < nw; ++i) {
                Word w = (*S)[i];
                while (w) {
                    const int u = i * 64 + __builtin_ctzll(w);
                    w &= w - 1;
                    const int c = and_count(row(u), P.data());
                    if (c > max_deg) { max_deg = c; pivot = u; }
                }
            }
        }

        std::vector<Word> cand(nw), NP(nw), NX(nw);
        for (int i = 0; i < nw; ++i)
            cand[i] = (pivot == -1) ? P[i] : (P[i] & ~row(pivot)[i]);

        for (int i = 0; i < nw; ++i) {
            while (cand[i]) {
                const int b = __builtin_ctzll(cand[i]);
                const int v = i * 64 + b;
                cand[i] &= cand[i] - 1;
                const Word* r = row(v);
                for (int j = 0; j < nw; ++j) { NP[j] = P[j] & r[j]; NX[j] = X[j] & r[j]; }
                R.push_back(v);
                recurse(NP, NX);
                R.pop_back();
                P[i] &= ~(Word{1} << b);
                X[i] |= (Word{1} << b);
            }
        }
    }
};

} // namespace

std::vector<int32_t> bron_kerbosch_maximum_bitset(const AdjList& G,
                                                  const std::vector<int32_t>& verts_sorted,
                                                  int initial_best_size) {
    BitBK s;
    s.n = static_cast<int>(verts_sorted.size());
    if (s.n <= initial_best_size) return {};
    s.nw = (s.n + 63) / 64;
    s.labels = verts_sorted;
    s.init_best = static_cast<std::size_t>(std::max(0, initial_best_size));
    s.adj.assign(static_cast<std::size_t>(s.n) * s.nw, 0);

    // map G label -> local index via binary search on the sorted vertex list
    for (int i = 0; i < s.n; ++i) {
        for (int32_t y : G[static_cast<std::size_t>(verts_sorted[i])]) {
            auto it = std::lower_bound(verts_sorted.begin(), verts_sorted.end(), y);
            if (it != verts_sorted.end() && *it == y) {
                const int j = static_cast<int>(it - verts_sorted.begin());
                s.adj[static_cast<std::size_t>(i) * s.nw + j / 64] |= Word{1} << (j % 64);
            }
        }
    }

    std::vector<Word> P(s.nw, 0), X(s.nw, 0);
    for (int v = 0; v < s.n; ++v) P[v / 64] |= Word{1} << (v % 64);
    s.recurse(P, X);
    g_bk_nodes.fetch_add(s.nodes, std::memory_order_relaxed);

    std::vector<int32_t> out;
    if (s.best.size() > s.init_best)
        for (int v : s.best) out.push_back(s.labels[static_cast<std::size_t>(v)]);
    return out;
}

} // namespace lazymc

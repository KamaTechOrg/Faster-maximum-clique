# LazyMC — bitset variants on the paper's graphs

Machine: cloud container, 4 cores, 15 GB RAM. Release build, `bench_cli --solver lazymc`.
Each configuration run 3 times (mean shown); time excludes file loading. Timeout: 600 s per run.

Configurations:
- **A** original
- **B** `LAZYMC_BITSET=1` (bitset Bron–Kerbosch)
- **C** `LAZYMC_BITSET=1 LAZYMC_BITSET_KVC=1` (bitset BK + bitset k-Vertex-Cover)

| Graph | n | m | ω (paper) | ω (all configs) | A (ms) | B (ms) | C (ms) | C vs A | BK ms A→C | BK nodes | slowest stage |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Bio-HS-CX | 4,413 | 108,818 | 86 | 86 | 57.4 | 56.2 | 55.8 | ×1.03 (noise) | 0 (BK not used) | 0 | DegreeBasedHeuristic (50 ms) |
| RoadNet-CA | 1,965,206 | 2,766,607 | 4 | 4 | 413 | 424 | 415 | ×1.0 (noise) | 0 (BK not used) | 0 | KCore (165 ms) |
| YouTube | 1,134,890 | 2,987,624 | 17 | 17 | 1,209 | 899 | 950 | ×1.3 | 1,144 → 192 | ~56K | SystematicSearch (580 → 320 ms) |
| LiveJournal | 4,846,609 | 42,851,237 | 321 | 321 | **> 600,000 (timeout ×3)** | **37,487** | **37,499** | **> ×16** | — → 32,000 | ~15.6M | SystematicSearch (31.9 s) |

All runs: `valid=yes`; clique sizes identical across configurations and equal to the paper.

## Observations
- **LiveJournal**: the original BK does not finish in 10 minutes; the bitset BK finishes in ~37 s.
  The incumbent after the heuristics is only 86 (true ω = 321), so the search runs ~15.6M BK nodes
  — 85% of the run is still BK. The paper reports 20.9 s on its hardware; remaining gap is in BK.
- **YouTube**: BK time drops ×6 (1,144 → 192 ms); overall ×1.3 because the heuristics and k-core
  (~430 ms) are untouched.
- **RoadNet-CA, Bio-HS-CX**: the search phase is a few ms; time is in the degree heuristic / k-core,
  which bitsets do not touch. No change, as expected.
- **k-VC bitset** has no effect on these graphs: k-VC takes ≤ 0.3 s even on LiveJournal.
  It matters on dense synthetic graphs (×3.8 on G(200,0.75)).
- Next bottleneck: on LiveJournal, the BK search itself (better initial incumbent / ordering would
  shrink the 15.6M nodes); on sparse graphs, DegreeBasedHeuristic and KCore (memory bound).

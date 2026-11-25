# Re-running the paper's Table II comparison

All solvers of Table II of *"Less is More: Faster Maximum Clique Search by Work-Avoidance"*
(PMC, dOmega-LS, dOmega-BS, MC-BRB, LazyMC), plus **LazyMC-bitset** (this branch:
`LAZYMC_BITSET=1 LAZYMC_BITSET_KVC=1`), on 21 of the paper's 28 graphs.

**Machine:** 4 cores Intel Xeon @ 2.3 GHz, 15 GB RAM (the paper: 128 threads).
PMC and LazyMC use all 4 cores; dOmega and MC-BRB are sequential (as in the paper).
Times in seconds, solver-reported, excluding graph loading. Timeout 600 s.
All solvers that finished agree on ω. Raw data: `results_comparison.csv`.
Reproduce: `bench/fetch_graphs.sh`, `bench/run_comparison.sh`, `bench/make_report.py`.

## Results on this machine (fastest in bold)

| Graph | ω | PMC | dOmega-LS | dOmega-BS | MC-BRB | LazyMC | LazyMC-bitset | fastest | bitset vs LazyMC |
|---|---|---|---|---|---|---|---|---|---|
| USAroad | 4 | 25.531 | 10.463 | 10.833 | **1.562** | 5.863 | 5.947 | MC-BRB | 0.99x |
| CAroad | 4 | 1.410 | 0.658 | 0.654 | **0.124** | 0.365 | 0.424 | MC-BRB | 0.86x |
| soflow | 55 | 47.309 | 62.184 | 41.003 | **5.488** | 116.733 | 25.706 | MC-BRB | 4.54x |
| talk | 26 | 2.626 | 39.420 | 13.875 | **0.628** | 12.905 | 3.071 | MC-BRB | 4.20x |
| patents | 11 | 25.318 | 4.403 | 4.635 | **1.742** | 3.380 | 3.061 | MC-BRB | 1.10x |
| LiveJournal | 321 | 33.418 | 5.983 | 4.456 | **1.251** | T.O. | 37.568 | MC-BRB | solved / T.O. |
| hudong | 267 | 8.886 | 1.159 | 1.450 | **0.603** | 2.216 | 2.509 | MC-BRB | 0.88x |
| flickr | 98 | 95.663 | T.O. | T.O. | **36.456** | T.O. | T.O. | MC-BRB | - |
| topcats | 39 | 21.418 | 8.538 | 9.154 | **2.764** | 19.812 | 11.165 | MC-BRB | 1.77x |
| pokec | 29 | 19.520 | 11.181 | 11.905 | **2.217** | 7.402 | 6.902 | MC-BRB | 1.07x |
| dblp | 114 | 0.344 | 0.075 | 0.093 | **0.011** | 0.080 | 0.084 | MC-BRB | 0.95x |
| orkut | 51 | 157.867 | OOM | OOM | **28.802** | 354.972 | 91.029 | MC-BRB | 3.90x |
| it | 432 | 1.087 | 0.157 | 0.155 | **0.032** | 1.049 | 1.092 | MC-BRB | 0.96x |
| hollywood | 2209 | 19.608 | 2.405 | 2.245 | **0.965** | 38.548 | 37.670 | MC-BRB | 1.02x |
| higgs | 71 | 7.626 | 10.228 | 10.689 | **1.899** | 59.023 | 10.317 | MC-BRB | 5.72x |
| uk | 500 | 1.060 | 0.142 | 0.128 | **0.038** | 0.158 | 0.169 | MC-BRB | 0.93x |
| WormNet | 121 | 0.365 | 0.699 | 0.665 | **0.035** | 1.703 | 1.012 | MC-BRB | 1.68x |
| HS-CX | 86 | 0.032 | 0.009 | 0.006 | **0.005** | 0.059 | 0.057 | MC-BRB | 1.04x |
| mouse | 561 | 40.119 | T.O. | T.O. | **16.919** | T.O. | T.O. | MC-BRB | - |
| human-1 | 1335 | T.O. | T.O. | 130.927 | **39.304** | T.O. | T.O. | MC-BRB | - |
| human-2 | 1300 | 502.862 | T.O. | 68.681 | **22.215** | T.O. | T.O. | MC-BRB | - |

Fastest-solver count: PMC: 0, dOmega-LS: 0, dOmega-BS: 0, MC-BRB: 21, LazyMC: 0, LazyMC-bitset: 0

### Paper's Table II (128 threads for PMC/LazyMC), same graphs

| Graph | PMC | dOmega-LS | dOmega-BS | MC-BRB | LazyMC |
|---|---|---|---|---|---|
| USAroad | 6.657 | 4.511 | 4.575 | 1.051 | 0.849 |
| CAroad | 0.161 | 0.292 | 0.325 | 0.162 | 0.127 |
| soflow | 10.339 | 42.182 | 43.115 | 4.877 | 0.510 |
| talk | 0.976 | 5.274 | 3.541 | 1.144 | 0.402 |
| patents | 1.683 | 2.236 | 2.132 | 1.207 | 0.260 |
| LiveJournal | 0.826 | 2.399 | 1.799 | 1.232 | 0.354 |
| hudong | 0.411 | 0.496 | 0.533 | 0.616 | 0.138 |
| flickr | T.O. | T.O. | 1412.050 | 34.225 | 475.045 |
| topcats | 3.719 | 10.595 | 10.813 | 2.329 | 0.313 |
| pokec | 1.679 | 10.022 | 10.482 | 1.826 | 0.215 |
| dblp | 0.084 | 0.072 | 0.049 | 0.020 | 0.048 |
| orkut | 13.021 | 189.173 | 185.938 | 19.660 | 1.774 |
| it | 0.077 | 0.063 | 0.063 | 0.041 | 0.053 |
| hollywood | 1.056 | 0.837 | 0.834 | 0.634 | 1.259 |
| higgs | 1.244 | 11.009 | 13.549 | 2.399 | 0.488 |
| uk | 0.056 | 0.056 | 0.057 | 0.039 | 0.041 |
| WormNet | 0.357 | 1.840 | 1.056 | 0.064 | 0.055 |
| HS-CX | 0.051 | 0.254 | 0.088 | 0.016 | 0.035 |
| mouse | 0.027 | T.O. | T.O. | 17.460 | 24.361 |
| human-1 | T.O. | 146.883 | 16.888 | 45.521 | 19.462 |
| human-2 | 86.392 | 65.854 | 8.932 | 27.328 | 11.571 |

OOM = out of memory (15 GB).

## Graphs not run
sinaweibo, friendster, webcc, uk-union, dimacs (too large for 15 GB RAM / 48 GB disk), yahoo
(Yahoo Webscope, needs an account), warwiki (no public copy found).

## Conclusions
- **MC-BRB is the fastest solver on all 21 graphs on this machine.** On 4 cores, LazyMC's
  advantage from the paper (which relies on 128 threads) disappears.
- **LazyMC-bitset vs the original LazyMC:** up to 5.7x faster (higgs 5.7x, soflow 4.5x,
  talk 4.2x, orkut 3.9x, topcats 1.8x, WormNet 1.7x), and LiveJournal is solved in 38 s
  where the original times out. On graphs where BK/k-VC are not the bottleneck it is within ±15 %.
- mouse, human-1, human-2, flickr (dense, large ω) still time out for both LazyMC versions;
  MC-BRB solves them in 17-40 s. This is where a colouring-bound search would be needed.

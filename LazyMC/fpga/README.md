# FPGA clique engine — prototype

Dedicated logic for the dense search stage of LazyMC. No physical board was used:
the design is verified by cycle-accurate simulation and placed & routed for a real
device (Lattice ECP5-85F) with the open-source flow (Yosys + nextpnr-ecp5).

## Algorithm
Colouring-bound branch and bound with binary branching (same as `model/mc_model.cpp`):

```
node(P, L): greedy colouring of P -> k colours, last coloured vertex v
            if L + k <= best: pop
            push continuation P \ {v}; R[L] = v; L++
            NP = P & N(v); NP == 0 ? (record L if better; pop) : node(NP, L)
```
Each stack level stores one N-bit row, so the whole stack fits on chip.
Vertex order is chosen on the host (`--order degen`: degeneracy order, highest core first).

## Dedicated gates (`rtl/clique_engine.v`)
| Gate | Function | Hardware | Time |
|---|---|---|---|
| adjacency gate | `NP = P & N(v)` | N parallel AND, row of adjacency matrix from BRAM | 1 cycle for N adjacency tests |
| colour-class gate | `Q & ~N(v)` | N parallel AND-NOT | 1 cycle |
| next vertex | lowest set bit of Q (`rtl/ffs.v`) | log2(N)-level mux tree | same cycle |
| empty test | `U == 0`, `NP == 0` | N-input OR tree | same cycle |
| bound | `L + k <= best` | 11-bit add + compare | 1 cycle |
| stack | continuation set per level | D x N bit BRAM | 1 cycle |

Cost: colouring = 2 cycles per vertex + 1 per colour class; branching = 2 cycles.

## Verification (`sim/tb.sv`, Verilator)
63/63 tests pass (60 random graphs incl. dense ones + 3 LiveJournal subproblems):
best size, node count and **cycle count identical** to the C++ model; returned set checked to be a clique.

## Synthesis (ECP5-85F, `syn/run_syn.sh`)
| N (bits) | LUT4 | FF | BRAM (DP16KD) | Fmax |
|---|---|---|---|---|
| 256 | 4,750 (6%) | 1,645 | 17 / 208 | 44.9 MHz |
| 1024 | 17,420 (21%) | 6,267 | 87 / 208 | 34.8 MHz |

Critical path: Q register -> find-first-set tree -> BRAM address / bit clear.

## LiveJournal BK subproblems (the 3 calls that make up ~85 % of LazyMC's run time)
| Subproblem | Existing bitset BK (CPU, 1 thread) | Colouring algorithm, CPU | Colouring algorithm, FPGA (cycles / 34.8 MHz) |
|---|---|---|---|
| n=210, best 61 -> 66 | 626 nodes, 0.73 ms | 132 nodes, 0.095 ms | 23,271 cycles, 0.67 ms |
| n=421, best 175 -> 228 | 278K nodes, 459 ms | 456 nodes, 0.84 ms | 224,811 cycles, 6.5 ms |
| n=737, best 175 (proved optimal) | 168M nodes, 305 s | 1 node, 0.015 ms | 1,804 cycles, 0.05 ms |

## Conclusions
- The big win is **algorithmic**: colouring bound + degeneracy ordering, which already runs
  orders of magnitude faster than the current BK on the CPU.
- A single engine on a low-cost ECP5 (35-45 MHz, 2 cycles per vertex) is ~5-8x **slower**
  than one CPU core running the same bit-parallel algorithm on these sizes.
- To beat the CPU the hardware needs: a faster device (UltraScale+ class), a pipelined
  1-vertex-per-cycle colouring loop, and many engines in parallel (several subproblems /
  branches at once). These were not tested here.

## Reproduce
```
g++ -O3 -march=native -o mc_model model/mc_model.cpp
./mc_model <graph> --order degen --emit-hex out/g        # golden results + hex files
verilator --binary --timing -Wno-fatal -Wno-WIDTH -O3 --top-module tb sim/tb.sv rtl/clique_engine.v rtl/ffs.v
./obj_dir/Vtb +prefix=out/g                              # RTL vs model
syn/run_syn.sh 1024 10 512 9 out/syn1024                 # ECP5 place & route
```
`model/bk_driver.cpp` times the repository's bitset BK on the same dumped subproblem
(link with `build/liblazymc.a`).

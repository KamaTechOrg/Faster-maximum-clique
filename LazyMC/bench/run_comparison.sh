#!/bin/bash
# Runs every solver of the LazyMC paper's Table II, plus this repository's LazyMC
# (original and bitset variants), on the converted graphs.
# Usage: bench/run_comparison.sh <conv_dir> <competitors_dir> <out.csv> [timeout_s] [graphs...]
# Output CSV: graph,solver,clique,time_s,status   (time = solver-reported compute time,
# excluding graph loading; status = ok | TO | FAIL)
set -u
CONV=$1; COMP=$2; OUT=$3; TO=${4:-600}; shift 4 || shift $#
HERE=$(cd "$(dirname "$0")/.." && pwd)
BENCH=$HERE/build/bench_cli
GRAPHS=${*:-"HS-CX WormNet dblp uk it CAroad hudong higgs talk patents LiveJournal topcats pokec human-2 human-1 mouse soflow hollywood flickr orkut USAroad"}
[ -s "$OUT" ] || echo "graph,solver,clique,time_s,status" > "$OUT"

run() {  # run <timeout> cmd...  -> sets RUN_OUT, RUN_RC
    RUN_OUT=$(timeout "$1" "${@:2}" 2>&1); RUN_RC=$?
}
emit() { echo "$1,$2,$3,$4,$5" | tee -a "$OUT"; }

for g in $GRAPHS; do
  el=$CONV/$g.el; brb=$CONV/$g.brb
  [ -s "$el" ] || { echo "missing $g"; continue; }

  # PMC (inside bench_cli, all cores via OpenMP)
  run "$TO" "$BENCH" --input "$el" --solver pmc
  l=$(echo "$RUN_OUT" | grep '\[PMC\]' | tail -1)
  if [ $RUN_RC -eq 124 ]; then emit "$g" PMC - - TO
  elif [[ "$l" == *valid=yes* ]]; then
    emit "$g" PMC "$(echo "$l" | sed -E 's/.*clique_size=([0-9]+).*/\1/')" \
         "$(echo "$l" | sed -E 's/.*time_ms=([0-9.]+).*/\1/' | awk '{printf "%.3f", $1/1000}')" ok
  else emit "$g" PMC - - FAIL; fi

  # dOmega LS / BS (sequential, as in the paper); time = degeneracy + search
  for v in LS BS; do
    run "$TO" "$COMP/dOmega/$v/bin/dOmega" -e "$el" -m 1
    l=$(echo "$RUN_OUT" | grep -F "$el" | tail -1)
    if [ $RUN_RC -eq 124 ]; then emit "$g" "dOmega-$v" - - TO
    elif [ -n "$l" ]; then
      emit "$g" "dOmega-$v" "$(echo "$l" | awk '{print $10}')" "$(echo "$l" | awk '{printf "%.3f", $9+$11}')" ok
    else emit "$g" "dOmega-$v" - - FAIL; fi
  done

  # MC-BRB (sequential), Total Time in microseconds
  run "$TO" "$COMP/MC-BRB/MC-BRB" MC-BRB "$brb"
  l=$(echo "$RUN_OUT" | grep -E 'Max(imum)? Clique Size: [0-9]+.*Total Time' | tail -1)
  if [ $RUN_RC -eq 124 ]; then emit "$g" MC-BRB - - TO
  elif [ -n "$l" ]; then
    emit "$g" MC-BRB "$(echo "$l" | sed -E 's/.*Clique Size: ([0-9]+).*/\1/')" \
         "$(echo "$l" | sed -E 's/.*Total Time: ([0-9,]+).*/\1/' | tr -d , | awk '{printf "%.3f", $1/1e6}')" ok
  else emit "$g" MC-BRB - - FAIL; fi

  # LazyMC: original, and bitset BK + bitset k-VC
  for cfg in "LazyMC:0:0" "LazyMC-bitset:1:1"; do
    IFS=: read -r name b k <<< "$cfg"
    RUN_OUT=$(LAZYMC_BITSET=$b LAZYMC_BITSET_KVC=$k timeout "$TO" "$BENCH" --input "$el" --solver lazymc 2>&1); RUN_RC=$?
    l=$(echo "$RUN_OUT" | grep '\[LazyMC\]' | tail -1)
    if [ $RUN_RC -eq 124 ]; then emit "$g" "$name" - - TO
    elif [[ "$l" == *valid=yes* ]]; then
      emit "$g" "$name" "$(echo "$l" | sed -E 's/.*clique_size=([0-9]+).*/\1/')" \
           "$(echo "$l" | sed -E 's/.*time_ms=([0-9.]+).*/\1/' | awk '{printf "%.3f", $1/1000}')" ok
    else emit "$g" "$name" - - FAIL; fi
  done
done

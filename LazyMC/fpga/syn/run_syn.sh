#!/bin/bash
# Usage: syn/run_syn.sh N LOGN D LOGD outdir
# Synthesises top_ecp5 for Lattice ECP5-85F with Yosys and places/routes with nextpnr-ecp5.
set -e
N=$1; LOGN=$2; D=$3; LOGD=$4; OUT=$5
HERE=$(cd "$(dirname "$0")/.." && pwd)
mkdir -p "$OUT"
yosys -q -l "$OUT/yosys.log" -p "read_verilog $HERE/rtl/ffs.v $HERE/rtl/clique_engine.v $HERE/rtl/top_ecp5.v; \
  chparam -set N $N -set LOGN $LOGN -set D $D -set LOGD $LOGD top_ecp5; \
  synth_ecp5 -top top_ecp5 -json $OUT/top.json; stat" 
nextpnr-ecp5 --85k --package CABGA756 --json "$OUT/top.json" --freq 100 --timing-allow-fail \
  --report "$OUT/report.json" -l "$OUT/nextpnr.log" -q || true
grep -E "Max frequency|Info: Device utilisation" -A 12 "$OUT/nextpnr.log" | grep -E "Max frequency|TRELLIS_SLICE|TRELLIS_FF|DP16KD|LUT4|TRELLIS_COMB|MULT" | tail -8

#!/usr/bin/env python3
"""Builds a markdown comparison table from run_comparison.sh output and the paper's Table II.
Usage: make_report.py <comparison.csv> <paper_table2.csv>"""
import csv, sys
SOL = ["PMC", "dOmega-LS", "dOmega-BS", "MC-BRB", "LazyMC", "LazyMC-bitset"]
ours = {}
for r in csv.DictReader(open(sys.argv[1])):
    ours[(r["graph"], r["solver"])] = r
paper = {r["graph"]: r for r in csv.DictReader(open(sys.argv[2]))}
graphs = [g for g in paper if any((g, s) in ours for s in SOL)]
def t(r): return float(r["time_s"]) if r and r["status"] == "ok" else None
def fmt(x): return "T.O." if x is None else f"{x:.3f}"
print("| Graph | ω | " + " | ".join(SOL) + " | fastest | bitset vs LazyMC |")
print("|---|---|" + "---|" * (len(SOL) + 2))
wins = {s: 0 for s in SOL}
for g in graphs:
    ts = {s: t(ours.get((g, s))) for s in SOL}
    ok = {s: v for s, v in ts.items() if v is not None}
    best = min(ok, key=ok.get) if ok else "-"
    if ok: wins[best] += 1
    cells = [("**" + fmt(ts[s]) + "**") if s == best else fmt(ts[s]) for s in SOL]
    a, b = ts["LazyMC"], ts["LazyMC-bitset"]
    sp = f"{a/b:.2f}x" if a and b else ("solved / T.O." if b and not a else "-")
    print(f"| {g} | {paper[g]['omega']} | " + " | ".join(cells) + f" | {best} | {sp} |")
print("\nFastest-solver count:", ", ".join(f"{s}: {n}" for s, n in wins.items()))
print("\n### Paper's Table II (128 threads for PMC/LazyMC), same graphs\n")
print("| Graph | " + " | ".join(SOL[:5]) + " |")
print("|---|" + "---|" * 5)
for g in graphs:
    print(f"| {g} | " + " | ".join(paper[g][s] for s in SOL[:5]) + " |")

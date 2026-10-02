#!/usr/bin/env python3
"""Combine the per-sweep summary CSVs into one across-sweep summary.
Usage: combine.py summary.csv <holdout-index or -1> sweeps/*/benchmarkResults_*.csv
- summary.csv: one row per size and method: median of the sweep medians, min and max of the sweep
  medians, lowest P25 and highest P75 of the timed runs, and the pi estimates.
- summary_speedups.csv (named after summary.csv): time ratios computed inside each sweep, then
  their min, median and max across sweeps, and how many sweeps the GPU was faster in.
- holdout check: the sweep at <holdout-index> (0-based) is compared with the range of the others.
"""
import csv, math, statistics, sys

out, hold = sys.argv[1], int(sys.argv[2])
sweeps = [[{k: float(v) for k, v in r.items()} for r in csv.DictReader(open(p))] for p in sys.argv[3:]]
M = [("Device", "gpu"), ("Host", "one_thread"), ("OMP", "openmp")]
sizes = sorted({r["Samples"] for sw in sweeps for r in sw})
# index each sweep by size; a sweep that stopped at a smaller size simply has no row for the larger ones
sweeps = [{r["Samples"]: r for r in sw} for sw in sweeps]

rows = []
for n in sizes:
    have = [s for s in sweeps if n in s]
    for pre, name in M:
        meds = [s[n][pre + "_Median_s"] for s in have]
        rows.append({
            "Samples": int(n), "Method": name, "Sweeps": len(have), "Timed_Runs_Per_Sweep": int(have[0][n]["Num_Runs"]),
            "Median_Of_Sweep_Medians_s": statistics.median(meds),
            "Min_Sweep_Median_s": min(meds), "Max_Sweep_Median_s": max(meds),
            "Lowest_P25_s": min(s[n][pre + "_P25_s"] for s in have),
            "Highest_P75_s": max(s[n][pre + "_P75_s"] for s in have),
            "Pi_Median": statistics.median(s[n][pre + "_Pi_Median"] for s in have),
            "Pi_Min_Sweep_Median": min(s[n][pre + "_Pi_Median"] for s in have),
            "Pi_Max_Sweep_Median": max(s[n][pre + "_Pi_Median"] for s in have),
        })
with open(out, "w", newline="") as f:
    w = csv.DictWriter(f, fieldnames=list(rows[0].keys()), lineterminator="\n")
    w.writeheader()
    for r in rows:
        w.writerow({k: (f"{v:.9g}" if isinstance(v, float) else v) for k, v in r.items()})

def fmt(s):
    if s >= 1: return f"{s:.3f} s"
    if s >= 1e-3: return f"{s*1e3:.3f} ms"
    return f"{s*1e6:.2f} us"

print("| Samples | GPU | One CPU thread | OpenMP (16 threads) | One thread / GPU | pi (GPU) |")
print("|---:|---:|---:|---:|---:|---:|")
by = {(r["Samples"], r["Method"]): r for r in rows}
for n in sizes:
    n = int(n)
    g, h, o = by[(n, "gpu")], by[(n, "one_thread")], by[(n, "openmp")]
    cell = lambda r: f"{fmt(r['Median_Of_Sweep_Medians_s'])} ({fmt(r['Min_Sweep_Median_s'])}-{fmt(r['Max_Sweep_Median_s'])})"
    ratio = h["Median_Of_Sweep_Medians_s"] / g["Median_Of_Sweep_Medians_s"]
    lo = h["Min_Sweep_Median_s"] / g["Max_Sweep_Median_s"]
    hi = h["Max_Sweep_Median_s"] / g["Min_Sweep_Median_s"]
    print(f"| {n:,} | {cell(g)} | {cell(h)} | {cell(o)} | {ratio:.3g}x ({lo:.3g}-{hi:.3g}) | {g['Pi_Median']:.5f} |")

print("\nPer-sweep winner, GPU vs one thread (G = GPU median faster, T = thread faster):")
for n in sizes:
    have = [s for s in sweeps if n in s]
    print(f"{int(n):>13,}: " + " ".join("G" if s[n]["Device_Median_s"] < s[n]["Host_Median_s"] else "T" for s in have)
          + "   GPU vs OpenMP: " + " ".join("G" if s[n]["Device_Median_s"] < s[n]["OMP_Median_s"] else "O" for s in have))

# Per-sweep time ratios (computed inside each sweep, then min / median / max across sweeps)
if out.endswith(".csv"):
    ratio_out = out.replace(".csv", "_speedups.csv")
    with open(ratio_out, "w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["Samples", "Sweeps",
                    "OneThread_over_GPU_Min", "OneThread_over_GPU_Median", "OneThread_over_GPU_Max",
                    "OpenMP_over_GPU_Min", "OpenMP_over_GPU_Median", "OpenMP_over_GPU_Max",
                    "OneThread_over_OpenMP_Min", "OneThread_over_OpenMP_Median", "OneThread_over_OpenMP_Max",
                    "Sweeps_GPU_Faster_Than_OneThread", "Sweeps_GPU_Faster_Than_OpenMP"])
        for n in sizes:
            have = [s[n] for s in sweeps if n in s]
            tg = [r["Host_Median_s"] / r["Device_Median_s"] for r in have]
            og = [r["OMP_Median_s"] / r["Device_Median_s"] for r in have]
            to = [r["Host_Median_s"] / r["OMP_Median_s"] for r in have]
            g = lambda v: [f"{min(v):.4g}", f"{statistics.median(v):.4g}", f"{max(v):.4g}"]
            w.writerow([int(n), len(have)] + g(tg) + g(og) + g(to)
                       + [sum(x > 1 for x in tg), sum(x > 1 for x in og)])
    print("\nwrote", ratio_out)

if hold < 0: sys.exit(0)
print(f"\nHoldout: sweep {hold} median inside the min-max range of the other sweeps' medians")
others = [s for j, s in enumerate(sweeps) if j != hold]
inside = total = 0
for n in sizes:
    if n not in sweeps[hold]: continue
    for pre, name in M:
        v = sweeps[hold][n][pre + "_Median_s"]
        lo = min(s[n][pre + "_Median_s"] for s in others if n in s); hi = max(s[n][pre + "_Median_s"] for s in others if n in s)
        ok = lo <= v <= hi
        inside += ok; total += 1
        if not ok:
            d = (v / lo - 1) if v < lo else (v / hi - 1)
            print(f"  outside: {int(n):,} {name}: {fmt(v)} vs range {fmt(lo)}-{fmt(hi)} ({d:+.1%} beyond the edge)")
print(f"  {inside}/{total} inside")

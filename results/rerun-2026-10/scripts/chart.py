#!/usr/bin/env python3
# The subtitle names the hardware of the October 2026 run; edit it for another machine.
"""Log-log SVG chart of the benchmark summary CSV.
Usage: chart.py summary.csv gpu-benchmark.svg <largest size that fits in GPU memory, e.g. 1e8>
GPU points above that size are drawn hollow with a dashed connector (data did not fit in GPU memory).
"""
import csv, math, sys

# combined CSV (one row per size and method) -> one row per size with Device/Host/OMP columns
_pre = {"gpu": "Device", "one_thread": "Host", "openmp": "OMP"}
_by = {}
for r in csv.DictReader(open(sys.argv[1])):
    d = _by.setdefault(float(r["Samples"]), {"Samples": float(r["Samples"])})
    p = _pre[r["Method"]]
    d[p + "_Median_s"] = float(r["Median_Of_Sweep_Medians_s"])
    d[p + "_SweepMin_s"] = float(r["Min_Sweep_Median_s"])
    d[p + "_SweepMax_s"] = float(r["Max_Sweep_Median_s"])
    _sw = _by.setdefault("_sweeps", set()); _sw.add(int(r["Sweeps"]))
    nruns = r["Timed_Runs_Per_Sweep"]
_sw = sorted(_by.pop("_sweeps"))
nsweeps = f"{_sw[-1]} sweeps" + (f" ({_sw[0]} at 1B)" if len(_sw) > 1 else "")
rows = [_by[k] for k in sorted(_by)]
out = sys.argv[2]
mem_limit = float(sys.argv[3])

W, H = 875, 560
L, R, T, B = 84, 165, 120, 70          # plot margins
PW, PH = W - L - R, H - T - B
XMIN, XMAX = 0, 9                      # log10 samples
YMIN, YMAX = -8, 2                     # log10 seconds

def X(n): return L + (math.log10(n) - XMIN) / (XMAX - XMIN) * PW
def Y(s): return T + (YMAX - math.log10(s)) / (YMAX - YMIN) * PH

ylab = {-8: "10 ns", -7: "100 ns", -6: "1 µs", -5: "10 µs", -4: "100 µs", -3: "1 ms", -2: "10 ms", -1: "100 ms", 0: "1 s", 1: "10 s", 2: "100 s"}
xlab = {0: "1", 1: "10", 2: "100", 3: "1K", 4: "10K", 5: "100K", 6: "1M", 7: "10M", 8: "100M", 9: "1B"}

def fmt(s):
    if s >= 1: return f"{s:.2f} s"
    if s >= 1e-3: return f"{s*1e3:.2f} ms"
    if s >= 1e-6: return f"{s*1e6:.1f} µs"
    return f"{s*1e9:.0f} ns"

series = [  # (csv prefix, label, css class, marker)
    ("Device", "GPU (ArrayFire CUDA)", "s1", "circle"),
    ("Host", "One CPU thread", "s2", "square"),
    ("OMP", "16 CPU threads (OpenMP)", "s3", "diamond"),
]

def marker(kind, x, y, cls, hollow, tip):
    fill = "var(--surface)" if hollow else f"var(--{cls})"
    common = f'class="mk" fill="{fill}" stroke="var(--{cls})" stroke-width="{2 if hollow else 0}"'
    ring = 'paint-order="stroke"' if not hollow else ""
    if kind == "circle":
        shape = f'<circle cx="{x:.1f}" cy="{y:.1f}" r="4.5" {common}/>'
    elif kind == "square":
        shape = f'<rect x="{x-4:.1f}" y="{y-4:.1f}" width="8" height="8" rx="1" {common}/>'
    else:
        shape = f'<path d="M{x:.1f} {y-5.5:.1f} L{x+5.5:.1f} {y:.1f} L{x:.1f} {y+5.5:.1f} L{x-5.5:.1f} {y:.1f} Z" {common}/>'
    halo = f'<circle cx="{x:.1f}" cy="{y:.1f}" r="7" fill="var(--surface)"/>'
    return f'<g>{halo}{shape}<title>{tip}</title></g>'

s = []
s.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" role="img" aria-labelledby="t d" font-family="system-ui, -apple-system, \'Segoe UI\', sans-serif">')
s.append('''<style>
svg { --surface:#fcfcfb; --ink:#0b0b0b; --ink2:#52514e; --muted:#6f6d68; --grid:#e1e0d9; --axis:#c3c2b7;
      --s1:#2a78d6; --s2:#eb6834; --s3:#1baf7a; }
@media (prefers-color-scheme: dark) {
  svg { --surface:#1a1a19; --ink:#ffffff; --ink2:#c3c2b7; --muted:#a3a29b; --grid:#2c2c2a; --axis:#383835;
        --s1:#3987e5; --s2:#d95926; --s3:#199e70; }
}
text { fill: var(--ink2); font-size: 12px; }
.title { fill: var(--ink); font-size: 16px; font-weight: 600; }
.sub { fill: var(--ink2); font-size: 12px; }
.tick { fill: var(--muted); font-size: 11px; font-variant-numeric: tabular-nums; }
.lab { fill: var(--ink); font-size: 12px; font-weight: 500; }
.note { fill: var(--ink2); font-size: 11px; }
</style>''')
s.append(f'<title id="t">Time to estimate pi against number of samples, log-log</title>')
crossings = []
for a, b in zip(rows, rows[1:]):
    if a["Device_Median_s"] >= a["Host_Median_s"] and b["Device_Median_s"] < b["Host_Median_s"]:
        x0, x1 = math.log10(a["Samples"]), math.log10(b["Samples"])
        y0 = math.log10(a["Host_Median_s"] / a["Device_Median_s"])
        y1 = math.log10(b["Host_Median_s"] / b["Device_Median_s"])
        xc = x0 + (0 - y0) * (x1 - x0) / (y1 - y0)
        tc = 10 ** (math.log10(a["Device_Median_s"]) + (xc - x0) * (math.log10(b["Device_Median_s"]) - math.log10(a["Device_Median_s"])) / (x1 - x0))
        crossings.append((a["Samples"], b["Samples"], 10 ** xc, tc))
desc = f"Median across {nsweeps} of {nruns} timed runs per size on an NVIDIA GeForce RTX 4060 and an AMD Ryzen 9 5950X under WSL2. "
if crossings:
    lo, hi, xc, _ = crossings[0]
    desc += f"The GPU overtakes one CPU thread between {int(lo):,} and {int(hi):,} samples."
s.append(f'<desc id="d">{desc}</desc>')
s.append(f'<rect width="{W}" height="{H}" fill="var(--surface)"/>')
s.append(f'<text class="title" x="{L}" y="30">Time to estimate pi, by number of samples</text>')
s.append(f'<text class="sub" x="{L}" y="50">Median across {nsweeps} of {nruns} timed runs after a warm-up call · whiskers: range of sweep medians · log-log</text>')
s.append(f'<text class="sub" x="{L}" y="68">NVIDIA GeForce RTX 4060 (8 GB) · AMD Ryzen 9 5950X, 16 logical CPUs · WSL2 · ArrayFire 3.9.0 · GCC 13.3 -O3</text>')

# grid + ticks
for e in range(YMIN, YMAX + 1):
    y = Y(10 ** e)
    s.append(f'<line x1="{L}" x2="{L+PW}" y1="{y:.1f}" y2="{y:.1f}" stroke="var(--grid)" stroke-width="1"/>')
    s.append(f'<text class="tick" x="{L-8}" y="{y+4:.1f}" text-anchor="end">{ylab[e]}</text>')
for e in range(XMIN, XMAX + 1):
    x = X(10 ** e)
    s.append(f'<line x1="{x:.1f}" x2="{x:.1f}" y1="{T}" y2="{T+PH}" stroke="var(--grid)" stroke-width="1"/>')
    s.append(f'<text class="tick" x="{x:.1f}" y="{T+PH+18}" text-anchor="middle">{xlab[e]}</text>')
s.append(f'<line x1="{L}" x2="{L+PW}" y1="{T+PH}" y2="{T+PH}" stroke="var(--axis)" stroke-width="1"/>')
s.append(f'<line x1="{L}" x2="{L}" y1="{T}" y2="{T+PH}" stroke="var(--axis)" stroke-width="1"/>')
s.append(f'<text class="lab" x="{L+PW/2}" y="{H-22}" text-anchor="middle">Samples (random points)</text>')
s.append(f'<text class="lab" transform="translate(20 {T+PH/2}) rotate(-90)" text-anchor="middle">Time per estimate (median)</text>')

# memory-limit region
xm = X(10 ** XMAX) - 8
s.append(f'<text class="note" x="{xm:.1f}" y="{T+16}" text-anchor="end">Hollow point: 1 billion samples need 8 GB,</text>')
s.append(f'<text class="note" x="{xm:.1f}" y="{T+30}" text-anchor="end">more than this 8 GB card had free</text>')

# crossover marker
for lo, hi, xc, tc in crossings[:1]:
    x, y = X(xc), Y(tc)
    s.append(f'<line x1="{x:.1f}" x2="{x:.1f}" y1="{y+10:.1f}" y2="{T+PH}" stroke="var(--ink2)" stroke-width="1"/>')
    s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="9" fill="none" stroke="var(--ink)" stroke-width="1.5"/>')
    s.append(f'<text class="lab" x="{x+12:.1f}" y="{y+95:.1f}">GPU overtakes one CPU thread</text>')
    s.append(f'<text class="note" x="{x+12:.1f}" y="{y+110:.1f}">between {int(lo):,} and {int(hi):,} samples</text>')

# series
for pre, label, cls, mk in series:
    pts = [(r["Samples"], r[pre + "_Median_s"], r[pre + "_SweepMin_s"], r[pre + "_SweepMax_s"]) for r in rows]
    is_gpu = pre == "Device"
    inmem = [p for p in pts if not (is_gpu and p[0] > mem_limit)]
    over = [p for p in pts if is_gpu and p[0] > mem_limit]
    d = " ".join(("M" if i == 0 else "L") + f"{X(n):.1f} {Y(t):.1f}" for i, (n, t, _, _) in enumerate(inmem))
    s.append(f'<path d="{d}" fill="none" stroke="var(--{cls})" stroke-width="2" stroke-linejoin="round" stroke-linecap="round"/>')
    if over:
        n0, t0 = inmem[-1][0], inmem[-1][1]
        for n, t, _, _ in over:
            s.append(f'<path d="M{X(n0):.1f} {Y(t0):.1f} L{X(n):.1f} {Y(t):.1f}" fill="none" stroke="var(--{cls})" stroke-width="2" stroke-dasharray="5 4" stroke-linecap="round"/>')
            n0, t0 = n, t
    for n, t, lo, hi in pts:
        s.append(f'<line x1="{X(n):.1f}" x2="{X(n):.1f}" y1="{Y(lo):.1f}" y2="{Y(hi):.1f}" stroke="var(--{cls})" stroke-width="2" stroke-linecap="round"/>')
    for n, t, lo, hi in pts:
        hollow = is_gpu and n > mem_limit
        tip = f"{label}, {int(n):,} samples: median {fmt(t)} (sweep medians: {fmt(lo)} to {fmt(hi)})" + (" — exceeds GPU memory" if hollow else "")
        s.append(marker(mk, X(n), Y(t), cls, hollow, tip))

# direct end labels (text in ink; colored key beside it)
ends = []
for pre, label, cls, mk in series:
    last = rows[-1]
    ends.append([Y(last[pre + "_Median_s"]), label, cls, mk])
ends.sort()
for i in range(1, len(ends)):              # keep labels at least 16px apart
    if ends[i][0] - ends[i - 1][0] < 16: ends[i][0] = ends[i - 1][0] + 16
short = {"GPU (ArrayFire CUDA)": "GPU", "One CPU thread": "1 CPU thread", "16 CPU threads (OpenMP)": "OpenMP, 16 threads"}
for y, label, cls, mk in ends:
    x = L + PW + 12
    s.append(f'<line x1="{x}" x2="{x+12}" y1="{y:.1f}" y2="{y:.1f}" stroke="var(--{cls})" stroke-width="2"/>')
    s.append(f'<text class="lab" x="{x+16}" y="{y+4:.1f}">{short[label]}</text>')

# legend (always present for >= 2 series)
lx = L
for pre, label, cls, mk in series:
    s.append(f'<line x1="{lx}" x2="{lx+18}" y1="{T-22}" y2="{T-22}" stroke="var(--{cls})" stroke-width="2"/>')
    s.append(marker(mk, lx + 9, T - 22, cls, False, label))
    s.append(f'<text x="{lx+24}" y="{T-18}">{label}</text>')
    lx += 34 + 7 * len(label)

s.append('</svg>')
open(out, "w").write("\n".join(s) + "\n")
print("wrote", out, "crossings", crossings)

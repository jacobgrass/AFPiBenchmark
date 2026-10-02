# What is this?
This tool is intended to be used to benchmark performance of different GPUs and CPUs by running a monte carlo
simulation of pi.

# How do I run it?

Pull the repo then 
`sudo ./installBuildRun.sh`

To run one backend by hand after `./build.sh`:
`./build/AFPiBenchMark_cuda [device] [timed runs per size] [largest power of ten]` (defaults: 0, 20, 9).
For each size, each method gets one untimed warm-up call, then its timed runs back to back. The results CSV holds the median,
quartiles, minimum and maximum time per method; a second CSV holds every timed run; the systemConfig file
records the ArrayFire version and backend, device, CPU, compiler and build flags.

Results of the October 2026 run on one GPU: `results/rerun-2026-10/RESULTS.md`.


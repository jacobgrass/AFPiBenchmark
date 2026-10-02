# Results: October 2026 run on one GPU

This folder holds one run of the benchmark on a single machine: an NVIDIA GeForce RTX 4060
(8 GB) and an AMD Ryzen 9 5950X, under WSL2 on Windows. It compares three ways to estimate pi by
Monte Carlo sampling, for 1 to 1 billion samples:

- **GPU**: ArrayFire on the CUDA backend (`pi_device`: `randu`, then `sum`).
- **One CPU thread**: a plain loop that draws points with the C library's `rand()` (`pi_host`).
- **16 CPU threads (OpenMP)**: an OpenMP loop over 16 threads, each with its own Mersenne Twister
  generator (`pi_omp`).

![Time to estimate pi, by number of samples](gpu-benchmark.svg)

## Method

- **Build**: `./build.sh` (CMake, Release build type). Compile line of the CUDA executable:
  `c++ -isystem <ArrayFire>/include -O3 -DNDEBUG -std=gnu++20 -fopenmp -c main.cpp`
  (GCC 13.3.0; the same flags for `BenchmarkTest.cpp`). Each `systemConfig_*.txt` records the
  compiler, build type and flags of the binary that produced it.
- **Timing**: for each size and each method, one untimed warm-up call, then 20 timed calls back
  to back, measured with `std::chrono::steady_clock`. The GPU call ends only after the count is
  copied back to the host (`sum<float>()`) and `af::sync()` returns, so ArrayFire's lazy evaluation
  and queued work are inside the timing.
- **Sweeps**: one sweep is one run of the program. Four sweeps cover 1 to 1 billion samples
  (`build/AFPiBenchMark_cuda 0 20 9`); six more cover 1 to 100 million (`... 0 20 8`), because
  the billion-sample stage takes about six of a sweep's seven minutes.
- **Machine load**: the machine is shared, so each sweep started only when the 1-minute load
  average was below 3, and `load_trace.txt` records it every 5 seconds. A sweep is used only if
  the load stayed below 4 (of 16 logical CPUs) outside its own last OpenMP stage; that stage's 16
  busy threads lift the average by about 3 on their own. Two sweeps that ran while other jobs
  pushed the load above 20 are kept in `excluded/` and are not used.
- **Reported numbers**: each sweep gives a median per size and method (its CSV also has the
  quartiles, minimum and maximum, and `benchmarkRuns_*.csv` has every timed call). `summary.csv`
  gives the median of the sweep medians and the lowest and highest sweep median.
  `summary_speedups.csv` gives time ratios computed inside each sweep, with their lowest, median
  and highest value across sweeps.

## Environment

| | |
|---|---|
| GPU | NVIDIA GeForce RTX 4060, 8 GB (8188 MiB), compute capability 8.9, power limit 115 W |
| Driver | NVIDIA 580.97 on the Windows host (supports CUDA up to 13.0); ArrayFire uses the CUDA 12.2 runtime |
| CPU | AMD Ryzen 9 5950X; WSL2 exposes 16 logical CPUs (16 of the processor's 32 hardware threads); OpenMP used 16 threads |
| OS | Linux 6.6.87.2-microsoft-standard-WSL2 (x86_64) |
| Library | ArrayFire 3.9.0 (build b59a1ae53), CUDA backend, from the official Linux installer |
| Compiler | GCC 13.3.0, CMake 4.3.0, Release (`-O3 -DNDEBUG`), C++20, OpenMP |

## Results

Median of the sweep medians, with the lowest and highest sweep median in brackets. Ten sweeps
for each size up to 100 million samples, four at 1 billion.

| Samples | GPU | One CPU thread | 16 CPU threads (OpenMP) | One thread ÷ GPU |
|---:|---:|---:|---:|---:|
| 1 | 131 µs (92-300 µs) | 0.04 µs (0.03-0.05 µs) | 10.6 µs (9.4 µs-12.0 ms) | 0.0003 |
| 10 | 268 µs (112-357 µs) | 0.18 µs (0.12-0.21 µs) | 10.5 µs (9.8-10.8 µs) | 0.0007 |
| 100 | 212 µs (107-351 µs) | 1.07 µs (0.97-1.64 µs) | 10.6 µs (9.7-11.9 µs) | 0.008 |
| 1,000 | 194 µs (86-359 µs) | 10.3 µs (9.3-16.2 µs) | 11.2 µs (10.5-11.8 µs) | 0.08 |
| 10,000 | 167 µs (96-389 µs) | 101 µs (92-162 µs) | 17.5 µs (15.6-21.3 µs) | 0.94 |
| 100,000 | 157 µs (104-308 µs) | 1.01 ms (0.97-1.06 ms) | 91 µs (74-114 µs) | 6.5 |
| 1,000,000 | 126 µs (117-273 µs) | 10.0 ms (9.4-10.9 ms) | 0.85 ms (0.61-0.93 ms) | 79 |
| 10,000,000 | 0.80 ms (0.73-0.85 ms) | 101 ms (94-106 ms) | 8.6 ms (7.7-16.9 ms) | 126.5 |
| 100,000,000 | 7.18 ms (6.63-7.25 ms) | 1.01 s (0.94-1.03 s) | 84 ms (72-87 ms) | 141 |
| 1,000,000,000 | 4.40 s (4.04-4.58 s) | 9.92 s (9.50-10.38 s) | 0.69 s (0.64-0.70 s) | 2.3 |

The last column is the median of the per-sweep ratios (`summary_speedups.csv`).

### Where the GPU starts to win

- At 1,000 samples and below, one CPU thread was faster than the GPU in every sweep.
- At 10,000 samples the two were close: one thread was faster in 8 of 10 sweeps.
- From 100,000 samples up, the GPU beat one CPU thread in every sweep.

Below 100,000 samples a GPU call took between about 0.09 and 0.4 ms whatever the size. That fixed
cost (mostly launching work through the driver and copying the result back) is why one thread wins
small problems. The level of that cost changed from one sweep to the next (about 0.1 ms in some, about
0.3 ms in others), so the exact switch point moves within the 10,000-100,000 range.

### Largest measured speed-up

At 100 million samples the GPU took 6.6-7.2 ms and one CPU thread 0.94-1.03 s. Within each
sweep the GPU was 139 to 143 times faster (median 141). At 10 million samples the ratio was
123 to 134.

### 16 CPU threads (OpenMP)

The OpenMP path gives correct estimates and is included. From 10 to 100,000 samples, 16 CPU
threads were faster than the GPU in every sweep; from 1 million to 100 million samples the GPU
was faster in every sweep, by 10.9 to 12.1 times at 100 million. The OpenMP loop uses a different
random number generator from the single-thread loop, so the ratio between those two CPU paths
(about 12 at 100 million samples) mixes the effect of threads with the effect of the generator.

### One billion samples

One billion samples need two 4 GB arrays on the GPU: 8 GB, more than the free memory of this
8 GB card. The run still completed (the Windows driver under WSL2 appears to place GPU data in system
memory when the card is full), but the GPU took 4.0-4.6 s: only 2.2 to 2.4 times faster than one thread, and
slower than 16 CPU threads (0.64-0.70 s) in every sweep. Problem sizes that fit in GPU memory are
the ones to compare.

### Accuracy

At 100 million samples the median estimate of every method was within 0.0001 of pi. ArrayFire's generator and
`rand()` start from fixed seeds, so the GPU and one-thread estimates are the same in every sweep;
the OpenMP threads seed from `std::random_device` and vary slightly.

## Limits of this run

- **One machine.** One consumer GPU (RTX 4060, 8 GB) and one desktop CPU. Other hardware will put
  the switch point and the speed-up elsewhere; the code is here to rerun.
- **WSL2.** The GPU is reached through the Windows driver. Per-call overhead under WSL2 can differ
  from native Linux, which affects small problem sizes most.
- **A small kernel.** Estimating pi is embarrassingly parallel and needs no input data copied to
  the GPU. Real workloads that move data between host and GPU, or that branch, will gain less.
- **The CPU baseline is a simple loop.** One CPU thread uses `rand()`, which is portable but not
  the fastest generator. A faster generator or vectorised CPU code would narrow the gap.
- **Busy GPU.** Each method's 20 calls run back to back after a warm-up call, so the GPU is timed
  while it is in use. In a separate check (`idle-check/`), about half of the 100-million-sample calls
  made after the GPU had been idle for one second or more took about 80 to 200 ms instead of about
  7 ms. The cause is not established, possibly a power-saving state. The sweeps do not time that case.

## Files

| Path | Content |
|---|---|
| `summary.csv` | per size and method: median of the sweep medians, lowest and highest sweep median, quartile extremes, pi |
| `summary_speedups.csv` | per size: time ratios inside each sweep (lowest, median, highest) and how many sweeps the GPU was faster in |
| `sweeps/NN_*/` | one folder per sweep: `benchmarkResults_*.csv` (median, quartiles, min, max), `benchmarkRuns_*.csv` (every timed call), `systemConfig_*.txt`, `environment.txt`, `load_trace.txt`, `stdout.log` |
| `excluded/` | two sweeps run while other jobs loaded the machine; not used |
| `gpu-benchmark.svg` | the chart above |
| `scripts/` | `run_sweep.sh` (one sweep plus environment and load record), `combine.py` (builds the two summary files), `chart.py` (draws the chart) |
| `idle-check/` | `idle_test.cpp` and its output: one 100-million-sample GPU call after 0 ms to 3 s of idle time |

## Rerun it

```bash
./build.sh                                    # needs ArrayFire 3.9 (CMAKE_PREFIX_PATH) and OpenMP
export AF_DIR=/opt/ArrayFire-3.9.0-Linux      # where ArrayFire is installed
results/rerun-2026-10/scripts/run_sweep.sh my-sweeps/01 9     # 1 to 1 billion samples
results/rerun-2026-10/scripts/run_sweep.sh my-sweeps/02 8     # 1 to 100 million samples
python3 results/rerun-2026-10/scripts/combine.py my-summary.csv -1 my-sweeps/*/benchmarkResults_*.csv
python3 results/rerun-2026-10/scripts/chart.py my-summary.csv my-chart.svg 1e8
```

The CUDA backend loads `libnvrtc-builtins` at run time, so ArrayFire's `lib64` folder must be on
`LD_LIBRARY_PATH` (`run_sweep.sh` sets it from `AF_DIR`; the repository's `installAF.sh` adds it to
the system library path instead).

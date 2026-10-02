#!/bin/bash
# Run one sweep of the CUDA benchmark and record the environment and machine load.
# Usage (from the repository root, after ./build.sh):
#   results/rerun-2026-10/scripts/run_sweep.sh <output-dir> [largest power of ten, default 9]
# ArrayFire is expected in $AF_DIR (default /opt/ArrayFire-3.9.0-Linux); its lib64 must be on
# LD_LIBRARY_PATH because the CUDA backend loads libnvrtc-builtins at run time.
set -u
OUT="$1"; MAXP="${2:-9}"
BIN="$(pwd)/build/AFPiBenchMark_cuda"
AF_DIR="${AF_DIR:-/opt/ArrayFire-3.9.0-Linux}"
export LD_LIBRARY_PATH="$AF_DIR/lib64:/usr/lib/wsl/lib:${LD_LIBRARY_PATH:-}"
mkdir -p "$OUT"; cd "$OUT" || exit 1
{
  echo "date_utc_start: $(date -u +%FT%TZ)"
  echo "loadavg_start: $(cat /proc/loadavg)"
  echo "uname: $(uname -srm)"
  echo "nvidia_smi: $(nvidia-smi --query-gpu=name,driver_version,memory.total,memory.used,power.limit,clocks.max.sm --format=csv,noheader)"
  nvidia-smi | sed -n '3p'
  echo "cpu: $(grep -m1 'model name' /proc/cpuinfo | cut -d: -f2-)"
  echo "nproc: $(nproc)"
  echo "gcc: $(g++ --version | head -1)"
  echo "cmake: $(cmake --version | head -1)"
  echo "arrayfire_dir: $AF_DIR"
  echo "command: build/AFPiBenchMark_cuda 0 20 $MAXP"
} > environment.txt
# 1-minute load average and GPU memory every 5 s while the sweep runs
( while true; do echo "$(date -u +%T) $(cut -d" " -f1-3 /proc/loadavg) gpu_mem_used=$(nvidia-smi --query-gpu=memory.used --format=csv,noheader)"; sleep 5; done ) > load_trace.txt &
SAMPLER=$!
"$BIN" 0 20 "$MAXP" > stdout.log 2> stderr.log
rc=$?
kill $SAMPLER
{
  echo "exit_code: $rc"
  echo "loadavg_end: $(cat /proc/loadavg)"
  echo "date_utc_end: $(date -u +%FT%TZ)"
} >> environment.txt
mv results/* . && rmdir results
exit $rc

#!/bin/bash

# Define the paths to the benchmarks
CPU_BENCHMARK="./build/AFPiBenchMark_cpu"
CUDA_BENCHMARK="./build/AFPiBenchMark_cuda"
OPENCL_BENCHMARK="./build/AFPiBenchMark_opencl"

# Check if the benchmark executables exist
if [[ ! -x "$CPU_BENCHMARK" ]]; then
    echo "Error: $CPU_BENCHMARK not found or not executable."
    exit 1
fi

if [[ ! -x "$CUDA_BENCHMARK" ]]; then
    echo "Error: $CUDA_BENCHMARK not found or not executable."
    exit 1
fi

if [[ ! -x "$OPENCL_BENCHMARK" ]]; then
    echo "Error: $OPENCL_BENCHMARK not found or not executable."
    exit 1
fi

# Run the CPU benchmark
echo "Running AFPiBenchMark_cpu..."
$CPU_BENCHMARK
if [[ $? -ne 0 ]]; then
    echo "AFPiBenchMark_cpu failed!"
    exit 1
fi
echo "AFPiBenchMark_cpu completed successfully."

# Run the CUDA benchmark
echo "Running AFPiBenchMark_cuda..."
$CUDA_BENCHMARK
if [[ $? -ne 0 ]]; then
    echo "AFPiBenchMark_cuda failed!"
    exit 1
fi
echo "AFPiBenchMark_cuda completed successfully."

# Run the OpenCL benchmark
echo "Running AFPiBenchMark_opencl..."
$OPENCL_BENCHMARK
if [[ $? -ne 0 ]]; then
    echo "AFPiBenchMark_opencl failed!"
    exit 1
fi
echo "AFPiBenchMark_opencl completed successfully."

echo "All benchmarks ran successfully."

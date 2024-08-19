#!/bin/bash
# Script to configure and build a CMake project

# Configure the project
cmake -B build -DCMSAKE_BUILD_TYPE=Release .


# Build the project
cmake --build build --config Release


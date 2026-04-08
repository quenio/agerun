#!/bin/bash
set -euo pipefail

# 1. Compile and link the benchmark
# Use the existing Makefile infrastructure to build the core library.
make build > /dev/null 2>&1

# Compile the benchmark with optimization and link against the library.
# Since the library is an archive file (libagerun.a) in bin/run-tests/, 
# we link directly against that for the benchmark.
gcc -O3 -Imodules -I. modules/ar_integration_performance_tests.c -Lbin/run-tests -lar_system -o bin/ar_integration_perf_benchmark

# 2. Run multiple times and report median to reduce noise
RUNS=5
SAMPLES=()

for i in $(seq 1 $RUNS); do
    # Run and capture only the METRIC line
    OUT=$(./bin/ar_integration_perf_benchmark | grep "METRIC total_µs=")
    VAL=$(echo $OUT | cut -d'=' -f2)
    SAMPLES+=($VAL)
done

# Calculate median
sorted_samples=($(printf '%s\n' "${SAMPLES[@]}" | sort -n))
median_index=$(( RUNS / 2 ))
median_val=${sorted_samples[$median_index]}

echo "METRIC total_µs=$median_val"

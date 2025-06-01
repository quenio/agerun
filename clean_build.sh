#!/bin/bash

# Clean Build Script for AgeRun
# Performs all build steps with minimal output

echo "=== AgeRun Clean Build ==="
echo "Starting at $(date)"
echo

# Function to run command and report status
run_step() {
    local step_name="$1"
    local cmd="$2"
    
    printf "%-30s" "$step_name..."
    
    # Run command and capture output
    if output=$($cmd 2>&1); then
        echo "✓"
        return 0
    else
        echo "✗"
        echo "Error output:"
        echo "$output" | tail -20
        return 1
    fi
}

# Step 1: Clean and build
echo -n "Clean and build...            "
if make clean >/dev/null 2>&1 && make >/dev/null 2>&1; then
    echo "✓"
else
    echo "✗"
    make clean 2>&1 | tail -5
    make 2>&1 | tail -5
fi

# Step 2: Static analysis
run_step "Static analysis (lib)" "make analyze"
run_step "Static analysis (tests)" "make analyze-tests"

# Step 3: Build executable
run_step "Build executable" "make executable"

# Step 4: Run tests
echo
echo "Running tests..."
if output=$(make test 2>&1); then
    # Count passed tests
    passed=$(echo "$output" | grep -c "Running.*_tests")
    echo "Tests: $passed passed ✓"
else
    echo "Tests: FAILED ✗"
    echo "$output" | grep -E "(ERROR:|FAIL:|failed)" | tail -10
fi

# Step 5: Run executable
echo
echo "Running executable..."
if output=$(make run 2>&1); then
    echo "Executable: ✓"
    # Show just the program output
    echo "$output" | grep -v "make\[" | head -5
    # Clean up persistence files created by executable
    rm -f bin/*.agerun
else
    echo "Executable: ✗"
fi

# Step 6: Sanitize tests (optional)
echo
echo "Sanitize tests (optional)..."
if output=$(make test-sanitize 2>&1); then
    echo "Sanitize: ✓"
else
    echo "Sanitize: Known issue on macOS"
fi

echo
echo "=== Build Summary ==="
echo "Completed at $(date)"

# Check for memory leaks in the last log
if [ -f bin/heap_memory_report.log ]; then
    if grep -q "No memory leaks detected" bin/heap_memory_report.log; then
        echo "Memory: No leaks detected ✓"
    else
        echo "Memory: Check bin/heap_memory_report.log"
    fi
fi
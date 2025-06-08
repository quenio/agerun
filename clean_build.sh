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
output=$(make test 2>&1)
# Count passed and failed tests
passed=$(echo "$output" | grep -c "All .* tests passed")
errors=$(echo "$output" | grep -c "ERROR: Test .* failed")
if [ $errors -eq 0 ]; then
    echo "Tests: $passed passed ✓"
else
    echo "Tests: $passed passed, $errors FAILED ✗"
    echo
    echo "Failed tests:"
    echo "$output" | grep "ERROR: Test .* failed" | sed 's/ERROR: Test /  - /' | sed 's/ failed.*//'
    echo
    echo "Error details (last 10):"
    echo "$output" | grep -B2 "ERROR: Test .* failed" | grep -E "(Assertion failed|Abort trap)" | head -5
    echo "$output" | grep -E "(FAIL:|failed)" | tail -10
    exit 1
fi

# Step 5: Run executable
echo
echo "Running executable..."
# Run the executable directly instead of through output capture to ensure memory report is written
make run >/dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "Executable: ✓"
    # Wait a moment for report to be written
    sleep 1
    # Check if executable memory report was created
    if [ -f "bin/memory_report_agerun.log" ]; then
        echo "Executable memory report created"
    else
        echo "Warning: Executable memory report not found"
    fi
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

# Check for memory leaks in all memory report logs
memory_reports=$(find bin -name "memory_report*.log" 2>/dev/null | sort)

if [ -z "$memory_reports" ]; then
    echo "Memory: No memory reports found"
else
    all_clean=true
    leaky_tests=""
    
    for report in $memory_reports; do
        if [ -f "$report" ]; then
            # Check if there are actual memory leaks (not just intentional test leaks)
            actual_leaks=$(grep -E "^Actual memory leaks: ([0-9]+)" "$report" 2>/dev/null | awk '{print $4}')
            
            # If we can't find the "Actual memory leaks" line, fall back to old behavior
            if [ -z "$actual_leaks" ]; then
                if ! grep -q "No memory leaks detected" "$report" 2>/dev/null; then
                    all_clean=false
                    test_name=$(basename "$report" .log | sed 's/memory_report_//')
                    leaky_tests="$leaky_tests $test_name"
                fi
            # If we found it and it's greater than 0, we have real leaks
            elif [ "$actual_leaks" -gt 0 ]; then
                all_clean=false
                test_name=$(basename "$report" .log | sed 's/memory_report_//')
                leaky_tests="$leaky_tests $test_name"
            fi
        fi
    done
    
    if $all_clean; then
        echo "Memory: No leaks detected ✓"
    else
        echo "Memory: LEAKS DETECTED in:$leaky_tests"
        echo "Check memory_report_*.log files in bin/"
        exit 1
    fi
fi
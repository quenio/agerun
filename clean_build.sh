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
echo -n "Static analysis (lib)...      "
output=$(make clean analyze 2>&1)
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "✓"
else
    echo "✗"
    # Check if it's due to static analysis findings
    if echo "$output" | grep -q "Static analysis FAILED:"; then
        echo "Static analysis found issues:"
        # Extract file-specific errors (e.g., "✗ 1 bugs found in modules/agerun_interpreter.c")
        echo "$output" | grep "✗.*bugs found in" | sed 's/^/  /'
        # Extract warnings with file:line:column format
        echo "$output" | grep -E "^modules/[a-zA-Z0-9_/.-]+\.[ch]:[0-9]+:[0-9]+: warning:" | head -10 | sed 's/^/  /'
        # Show total summary
        echo "$output" | grep "Static analysis FAILED:" | sed 's/^/  /'
    else
        echo "Build/analysis error:"
        echo "$output" | tail -20
    fi
fi

echo -n "Static analysis (tests)...    "
output=$(make clean analyze-tests 2>&1)
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "✓"
else
    echo "✗"
    # Check if it's due to static analysis findings
    if echo "$output" | grep -q "Static analysis FAILED:"; then
        echo "Static analysis found issues:"
        # Extract file-specific errors (e.g., "✗ 2 bugs found in methods/message_router_tests.c")
        echo "$output" | grep "✗.*bugs found in" | sed 's/^/  /'
        # Extract warnings with file:line:column format
        echo "$output" | grep -E "^(modules|methods)/[a-zA-Z0-9_/.-]+\.[ch]:[0-9]+:[0-9]+: warning:" | head -10 | sed 's/^/  /'
        # Show total summary
        echo "$output" | grep "Static analysis FAILED:" | sed 's/^/  /'
    else
        echo "Build/analysis error:"
        echo "$output" | tail -20
    fi
fi

# Step 3: Build executable
run_step "Build executable" "make clean executable"

# Step 4: Run tests
echo
echo "Running tests..."
output=$(make clean test 2>&1)
# Count passed and failed tests by counting test executables that ran
total_tests=$(echo "$output" | grep -c "^Running test: bin/")
passed=$(echo "$output" | grep -c "All .* tests passed")
errors=$(echo "$output" | grep -c "ERROR: Test .* failed")
aborts=$(echo "$output" | grep -c "Abort trap")
failed=$((errors + aborts))
if [ $failed -eq 0 ]; then
    echo "Tests: $total_tests run, all passed ✓"
else
    echo "Tests: $total_tests run, $passed passed, $failed FAILED ✗"
    echo
    echo "Failed tests:"
    echo "$output" | grep "ERROR: Test .* failed" | sed 's/ERROR: Test /  - /' | sed 's/ failed.*//'
    echo "$output" | grep -B1 "Abort trap" | grep "^Running test:" | sed 's/Running test: bin\//  - /' | sed 's/ (aborted)$//' | sort -u
    echo
    echo "Error details (last 10):"
    echo "$output" | grep -B2 "ERROR: Test .* failed" | grep -E "(Assertion failed|Abort trap)" | head -5
    echo "$output" | grep -E "(FAIL:|failed)" | grep -v "EXPECTED FAIL" | tail -10
    exit 1
fi

# Step 5: Run executable
echo
echo "Running executable..."
# Run the executable directly instead of through output capture to ensure memory report is written
make clean run >/dev/null 2>&1
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

# Step 6: Sanitize tests
echo
echo "Running sanitizer tests..."
output=$(make clean test-sanitize 2>&1)
sanitize_exit_code=$?

# Count sanitizer test runs
sanitize_total=$(echo "$output" | grep -c "^Running test: bin/")
sanitize_passed=$(echo "$output" | grep -c "All .* tests passed")

# Check if sanitizer tests failed to build
build_failed=$(echo "$output" | grep -c "Undefined symbols for architecture\|ld: symbol.* not found")

# Check for AddressSanitizer errors
asan_errors=$(echo "$output" | grep -c "ERROR: AddressSanitizer:")
heap_use_after_free=$(echo "$output" | grep -c "heap-use-after-free")
stack_buffer_overflow=$(echo "$output" | grep -c "stack-buffer-overflow")
heap_buffer_overflow=$(echo "$output" | grep -c "heap-buffer-overflow")
memory_leaks=$(echo "$output" | grep -c "ERROR: LeakSanitizer:")

if [ $build_failed -gt 0 ]; then
    echo "Sanitizer: Build failed (missing sanitizer runtime) ⚠️"
    echo "Note: This is a known issue on some macOS systems"
elif [ $sanitize_exit_code -eq 0 ] && [ $asan_errors -eq 0 ]; then
    echo "Sanitizer: $sanitize_total tests run, all passed ✓"
else
    echo "Sanitizer: $sanitize_total tests run, ERRORS DETECTED ✗"
    echo
    echo "Sanitizer Report:"
    echo "  - AddressSanitizer errors: $asan_errors"
    if [ $heap_use_after_free -gt 0 ]; then
        echo "    • Heap use-after-free: $heap_use_after_free"
    fi
    if [ $stack_buffer_overflow -gt 0 ]; then
        echo "    • Stack buffer overflow: $stack_buffer_overflow"
    fi
    if [ $heap_buffer_overflow -gt 0 ]; then
        echo "    • Heap buffer overflow: $heap_buffer_overflow"
    fi
    if [ $memory_leaks -gt 0 ]; then
        echo "    • Memory leaks detected: $memory_leaks"
    fi
    
    # Show first error details
    echo
    echo "First sanitizer error:"
    echo "$output" | grep -A10 "ERROR: AddressSanitizer:" | head -15
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
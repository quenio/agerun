#!/bin/bash

# Full Build Script for AgeRun
# Performs complete build with all tests, checks, and validations

# Check if running from repo root
if [ ! -f "Makefile" ] || [ ! -d "modules" ] || [ ! -d "methods" ]; then
    echo "ERROR: This script must be run from the AgeRun repository root directory."
    echo "Please run 'make full-build' from the repository root instead."
    exit 1
fi

echo "=== AgeRun Full Build ==="
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
        # Extract file-specific errors (e.g., "✗ 1 bugs found in modules/ar_interpreter.c")
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
test_exit_code=$?

# Check if tests failed to compile
if [ $test_exit_code -ne 0 ]; then
    # Check for compilation errors
    compile_errors=$(echo "$output" | grep -c "error:")
    if [ $compile_errors -gt 0 ]; then
        echo "Tests: COMPILATION FAILED ✗"
        echo
        echo "Compilation errors:"
        echo "$output" | grep -E "error:|^modules/.*_tests\.c:" | head -20
        exit 1
    fi
fi

# Count passed and failed tests by counting test executables that ran
total_tests=$(echo "$output" | grep -c "^Running test: bin/")
passed=$(echo "$output" | grep -c "All .* tests passed")
errors=$(echo "$output" | grep -c "ERROR: Test .* failed")
aborts=$(echo "$output" | grep -c "Abort trap")
failed=$((errors + aborts))

# If no tests ran but make succeeded, something is wrong
if [ $test_exit_code -eq 0 ] && [ $total_tests -eq 0 ]; then
    echo "Tests: WARNING - No tests were run ⚠️"
    echo "Check if test executables are being built properly"
elif [ $failed -eq 0 ] && [ $test_exit_code -eq 0 ]; then
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

# Check for memory leaks in all memory report logs
echo
echo "Checking for memory leaks..."
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

# Step 5: Run executable
echo
echo "Running executable..."
output=$(make clean run 2>&1)
exec_exit_code=$?
if [ $exec_exit_code -eq 0 ]; then
    echo "Executable: ✓"
    # Wait a moment for report to be written
    sleep 1
    # Check if executable memory report was created and check for leaks
    if [ -f "bin/memory_report_agerun.log" ]; then
        # Check for memory leaks in executable
        actual_leaks=$(grep -E "^Actual memory leaks: ([0-9]+)" "bin/memory_report_agerun.log" 2>/dev/null | awk '{print $4}')
        
        # If we can't find the "Actual memory leaks" line, fall back to old behavior
        if [ -z "$actual_leaks" ]; then
            if grep -q "No memory leaks detected" "bin/memory_report_agerun.log" 2>/dev/null; then
                echo "Executable memory: No leaks detected ✓"
            else
                echo "Executable memory: LEAKS DETECTED ✗"
                echo "Check bin/memory_report_agerun.log for details"
            fi
        # If we found it and it's greater than 0, we have real leaks
        elif [ "$actual_leaks" -gt 0 ]; then
            echo "Executable memory: LEAKS DETECTED ($actual_leaks leaks) ✗"
            echo "Check bin/memory_report_agerun.log for details"
        else
            echo "Executable memory: No leaks detected ✓"
        fi
    else
        echo "Warning: Executable memory report not found"
    fi
    # Clean up persistence files created by executable
    rm -f bin/*.agerun
else
    echo "Executable: ✗"
    echo "Error output:"
    echo "$output" | tail -10
fi

# Step 6: Sanitize tests (ASan + UBSan)
echo
echo "Running sanitizer tests (ASan + UBSan)..."
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

# Check for UndefinedBehaviorSanitizer errors
ubsan_errors=$(echo "$output" | grep -c "runtime error:")
integer_overflow=$(echo "$output" | grep -c "signed integer overflow")
array_bounds=$(echo "$output" | grep -c "index.*out of bounds")
null_pointer=$(echo "$output" | grep -c "null pointer")

if [ $build_failed -gt 0 ]; then
    echo "Sanitizer: Build failed (missing sanitizer runtime) ⚠️"
    echo "Note: This is a known issue on some macOS systems"
elif [ $sanitize_exit_code -eq 0 ] && [ $asan_errors -eq 0 ] && [ $ubsan_errors -eq 0 ]; then
    echo "Sanitizer: $sanitize_total tests run, all passed ✓"
else
    echo "Sanitizer: $sanitize_total tests run, ERRORS DETECTED ✗"
    echo
    echo "Sanitizer Report:"
    
    if [ $asan_errors -gt 0 ]; then
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
    fi
    
    if [ $ubsan_errors -gt 0 ]; then
        echo "  - UndefinedBehavior errors: $ubsan_errors"
        if [ $integer_overflow -gt 0 ]; then
            echo "    • Integer overflow: $integer_overflow"
        fi
        if [ $array_bounds -gt 0 ]; then
            echo "    • Array bounds violation: $array_bounds"
        fi
        if [ $null_pointer -gt 0 ]; then
            echo "    • Null pointer dereference: $null_pointer"
        fi
    fi
    
    # Show first error details
    echo
    if [ $asan_errors -gt 0 ]; then
        echo "First AddressSanitizer error:"
        echo "$output" | grep -A10 "ERROR: AddressSanitizer:" | head -15
    elif [ $ubsan_errors -gt 0 ]; then
        echo "First UndefinedBehavior error:"
        echo "$output" | grep -A2 "runtime error:" | head -5
    fi
fi

# Step 7: Run executable with ASan + UBSan
echo
echo "Running executable with sanitizers (ASan + UBSan)..."
output=$(make clean run-sanitize 2>&1)
exec_san_exit_code=$?

# Check for sanitizer errors in executable
exec_asan_errors=$(echo "$output" | grep -c "ERROR: AddressSanitizer:")
exec_ubsan_errors=$(echo "$output" | grep -c "runtime error:")

if [ $exec_san_exit_code -eq 0 ] && [ $exec_asan_errors -eq 0 ] && [ $exec_ubsan_errors -eq 0 ]; then
    echo "Sanitized Executable: ✓"
else
    echo "Sanitized Executable: ERRORS DETECTED ✗"
    if [ $exec_asan_errors -gt 0 ]; then
        echo "  - AddressSanitizer errors: $exec_asan_errors"
    fi
    if [ $exec_ubsan_errors -gt 0 ]; then
        echo "  - UndefinedBehavior errors: $exec_ubsan_errors"
    fi
    echo
    echo "First error:"
    echo "$output" | grep -E "(ERROR: AddressSanitizer:|runtime error:)" -A5 | head -10
fi

# Step 8: ThreadSanitizer tests
echo
echo "Running ThreadSanitizer tests..."
output=$(make clean test-tsan 2>&1)
tsan_exit_code=$?

# Count TSan test runs
tsan_total=$(echo "$output" | grep -c "^Running test: bin/")
tsan_passed=$(echo "$output" | grep -c "All .* tests passed")

# Check for ThreadSanitizer errors
tsan_errors=$(echo "$output" | grep -c "WARNING: ThreadSanitizer:")
data_races=$(echo "$output" | grep -c "data race")

if [ $tsan_exit_code -eq 0 ] && [ $tsan_errors -eq 0 ]; then
    echo "ThreadSanitizer: $tsan_total tests run, all passed ✓"
else
    echo "ThreadSanitizer: $tsan_total tests run, ERRORS DETECTED ✗"
    echo
    echo "ThreadSanitizer Report:"
    echo "  - Data races detected: $data_races"
    
    # Show first race details
    if [ $tsan_errors -gt 0 ]; then
        echo
        echo "First data race:"
        echo "$output" | grep -A15 "WARNING: ThreadSanitizer:" | head -20
    fi
fi

# Step 9: ThreadSanitizer executable
echo
echo "Running executable with ThreadSanitizer..."
output=$(make clean run-tsan 2>&1)
exec_tsan_exit_code=$?

# Check for TSan errors in executable
exec_tsan_errors=$(echo "$output" | grep -c "WARNING: ThreadSanitizer:")
exec_data_races=$(echo "$output" | grep -c "data race")

if [ $exec_tsan_exit_code -eq 0 ] && [ $exec_tsan_errors -eq 0 ]; then
    echo "TSan Executable: ✓"
else
    echo "TSan Executable: ERRORS DETECTED ✗"
    echo "  - Data races detected: $exec_data_races"
    if [ $exec_tsan_errors -gt 0 ]; then
        echo
        echo "First data race:"
        echo "$output" | grep -A10 "WARNING: ThreadSanitizer:" | head -15
    fi
fi

# Step 10: Check naming conventions
echo
echo "Checking naming conventions..."
if make check-naming >/dev/null 2>&1; then
    echo "Naming conventions: All conventions followed ✓"
else
    echo "Naming conventions: VIOLATIONS FOUND ✗"
    echo "Run 'make check-naming' for details"
fi

# Step 11: Check documentation
echo
echo "Checking documentation..."
if make check-docs >/dev/null 2>&1; then
    echo "Documentation: All checks passed ✓"
else
    echo "Documentation: ISSUES FOUND ✗"
    echo "Run 'make check-docs' for details"
fi

echo
echo "=== Build Summary ==="
echo "Completed at $(date)"
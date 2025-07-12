#!/bin/bash

# Build Script for AgeRun
# Performs complete build with all tests, checks, and validations in parallel

# Check if running from repo root
if [ ! -f "Makefile" ] || [ ! -d "modules" ] || [ ! -d "methods" ]; then
    echo "ERROR: This script must be run from the AgeRun repository root directory."
    echo "Please run 'make build' from the repository root instead."
    exit 1
fi

echo "=== AgeRun Build ==="
echo "Starting at $(date)"
echo

# Function to run a job and report completion
run_job() {
    local name="$1"
    local cmd="$2"
    local logfile="$3"
    
    # Run command in background, capturing output
    (
        # Ensure log directory exists
        mkdir -p "$(dirname "$logfile")"
        # Run command and capture output
        if $cmd > "$logfile" 2>&1; then
            exitcode=0
        else
            exitcode=$?
        fi
        echo $exitcode > "${logfile}.exitcode"
        if [ "$exitcode" -eq 0 ]; then
            echo "✓ $name completed successfully"
        else
            echo "✗ $name FAILED (exit code: $exitcode)"
        fi
    ) &
}

# Build from current state (no automatic clean)

# Create output directory for logs only
mkdir -p logs

# Launch all jobs in parallel
echo "Launching parallel builds..."
run_job "check-naming" "make check-naming" "logs/check-naming.log"
run_job "check-docs" "make check-docs" "logs/check-docs.log"
run_job "analyze-exec" "make analyze-exec" "logs/analyze-exec.log"
run_job "analyze-tests" "make analyze-tests" "logs/analyze-tests.log"
run_job "run-tests" "make run-tests" "logs/run-tests.log"
run_job "run-exec" "make run-exec" "logs/run-exec.log"
run_job "sanitize-tests" "make sanitize-tests" "logs/sanitize-tests.log"
run_job "sanitize-exec" "make sanitize-exec" "logs/sanitize-exec.log"
run_job "tsan-tests" "make tsan-tests" "logs/tsan-tests.log"
run_job "tsan-exec" "make tsan-exec" "logs/tsan-exec.log"

# Wait for all jobs to complete
wait

echo
echo "=== Build Results Summary ==="

# Function to display results for a category
show_results() {
    local category="$1"
    shift
    local logs=("$@")
    
    echo
    echo "--- $category ---"
    for log in "${logs[@]}"; do
        if [ -f "$log" ]; then
            name=$(basename "$log" .log)
            exitcode=$(cat "${log}.exitcode" 2>/dev/null || echo "999")
            if [ "$exitcode" = "0" ]; then
                echo "✓ $name: PASSED"
                # Show summary info for specific types
                case "$name" in
                    "run-tests"|"sanitize-tests"|"tsan-tests")
                        # Extract test counts
                        total_tests=$(grep -c "^Running test:" "$log" 2>/dev/null || echo "0")
                        passed_tests=$(grep -c "All .* tests passed" "$log" 2>/dev/null || echo "0")
                        if [ $total_tests -gt 0 ]; then
                            echo "  $total_tests tests run"
                        fi
                        ;;
                    "analyze-exec"|"analyze-tests")
                        # Check for analysis results
                        if grep -q "no bugs found" "$log" 2>/dev/null; then
                            echo "  No bugs found"
                        else
                            # Show scan-build warnings
                            grep -B2 "warning:" "$log" 2>/dev/null | grep -E "(warning:|\.c:|\.h:)" | head -10 | sed 's/^/    /'
                        fi
                        ;;
                    "check-naming")
                        if grep -q "All naming conventions followed" "$log" 2>/dev/null; then
                            echo "  All conventions followed"
                        fi
                        ;;
                    "check-docs")
                        if grep -q "All documentation checks passed" "$log" 2>/dev/null; then
                            echo "  All checks passed"
                        fi
                        ;;
                esac
            else
                echo "✗ $name: FAILED"
                # Show error summary
                echo "  Error details:"
                # Different error patterns for different tools
                case "$name" in
                    "run-tests"|"sanitize-tests"|"tsan-tests")
                        grep "ERROR: Test .* failed" "$log" 2>/dev/null | head -5 | sed 's/^/    /'
                        ;;
                    "analyze-exec"|"analyze-tests")
                        grep "bugs found in" "$log" 2>/dev/null | head -5 | sed 's/^/    /'
                        # Show the actual warnings from the log
                        echo "    Static analysis warnings:"
                        if grep -q "=== Static Analysis Warnings ===" "$log" 2>/dev/null; then
                            # New format with dedicated warnings section
                            sed -n '/=== Static Analysis Warnings ===/,/^$/p' "$log" 2>/dev/null | grep -v "=== Static Analysis Warnings ===" | head -20 | sed 's/^/      /'
                        else
                            # Fallback to old format
                            grep -B1 "warning:" "$log" 2>/dev/null | grep -E "(\.c:|\.h:|warning:)" | head -10 | sed 's/^/      /'
                        fi
                        ;;
                    "check-naming")
                        grep -E "(ERROR:|Warning:)" "$log" 2>/dev/null | head -5 | sed 's/^/    /'
                        ;;
                    "check-docs")
                        grep -E "(ERROR:|Invalid)" "$log" 2>/dev/null | head -5 | sed 's/^/    /'
                        ;;
                    *)
                        grep -E "(ERROR:|FAILED:|error:|failed)" "$log" 2>/dev/null | head -5 | sed 's/^/    /'
                        ;;
                esac
            fi
        fi
    done
}

# Display results in order: checks → analysis → runs → sanitizers → tsan
show_results "Code Quality Checks" "logs/check-naming.log" "logs/check-docs.log"
show_results "Static Analysis" "logs/analyze-exec.log" "logs/analyze-tests.log"
show_results "Build and Run" "logs/run-tests.log" "logs/run-exec.log"
show_results "Sanitizers (ASan + UBSan)" "logs/sanitize-tests.log" "logs/sanitize-exec.log"
show_results "Thread Sanitizer" "logs/tsan-tests.log" "logs/tsan-exec.log"

# Check for memory leaks across all directories
echo
echo "--- Memory Leak Check ---"
leak_found=false
leak_dirs=()

# Check each output directory for memory reports
for dir in bin/run-tests bin/run-exec bin/sanitize-tests bin/sanitize-exec bin/tsan-tests bin/tsan-exec; do
    if [ -d "$dir" ]; then
        memory_reports=$(find "$dir" -name "memory_report*.log" 2>/dev/null | sort)
        for report in $memory_reports; do
            if [ -f "$report" ]; then
                # Check if there are actual memory leaks
                actual_leaks=$(grep -E "^Actual memory leaks: ([0-9]+)" "$report" 2>/dev/null | awk '{print $4}')
                
                if [ -n "$actual_leaks" ] && [ "$actual_leaks" -gt 0 ]; then
                    leak_found=true
                    leak_dirs+=("$dir")
                    break
                elif [ -z "$actual_leaks" ] && ! grep -q "No memory leaks detected" "$report" 2>/dev/null; then
                    # Old format check
                    leak_found=true
                    leak_dirs+=("$dir")
                    break
                fi
            fi
        done
    fi
done

if $leak_found; then
    echo "✗ Memory leaks detected in:"
    printf '%s\n' "${leak_dirs[@]}" | sort -u | sed 's/^/  /'
else
    echo "✓ No memory leaks detected"
fi

# Check for sanitizer-specific issues
echo
echo "--- Sanitizer Summary ---"

# AddressSanitizer results
if [ -f "bin/sanitize-test.log" ] || [ -f "bin/sanitize-run.log" ]; then
    asan_errors=0
    for log in bin/sanitize-test.log bin/sanitize-run.log; do
        if [ -f "$log" ]; then
            errors=$(grep -c "ERROR: AddressSanitizer:" "$log" 2>/dev/null || echo "0")
            asan_errors=$((asan_errors + errors))
        fi
    done
    
    if [ $asan_errors -gt 0 ]; then
        echo "✗ AddressSanitizer: $asan_errors errors detected"
    else
        echo "✓ AddressSanitizer: No errors detected"
    fi
fi

# ThreadSanitizer results
if [ -f "bin/tsan-test.log" ] || [ -f "bin/tsan-run.log" ]; then
    tsan_errors=0
    for log in bin/tsan-test.log bin/tsan-run.log; do
        if [ -f "$log" ]; then
            errors=$(grep -c "WARNING: ThreadSanitizer:" "$log" 2>/dev/null || echo "0")
            tsan_errors=$((tsan_errors + errors))
        fi
    done
    
    if [ $tsan_errors -gt 0 ]; then
        echo "✗ ThreadSanitizer: $tsan_errors data races detected"
    else
        echo "✓ ThreadSanitizer: No data races detected"
    fi
fi

echo
echo "=== Build Summary ==="
echo "Completed at $(date)"

# Determine overall exit status
exit_status=0
for exitfile in logs/*.log.exitcode; do
    if [ -f "$exitfile" ]; then
        code=$(cat "$exitfile")
        if [ "$code" -ne 0 ]; then
            exit_status=1
        fi
    fi
done

if [ $exit_status -eq 0 ]; then
    echo "Overall status: ✓ SUCCESS"
else
    echo "Overall status: ✗ FAILURE"
    echo
    echo "Failed jobs:"
    for exitfile in logs/*.log.exitcode; do
        if [ -f "$exitfile" ]; then
            code=$(cat "$exitfile")
            if [ "$code" -ne 0 ]; then
                job=$(basename "$exitfile" .log.exitcode)
                echo "  - $job"
            fi
        fi
    done
fi

exit $exit_status
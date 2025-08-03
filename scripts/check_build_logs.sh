#!/bin/bash

# Script to check build logs for potential issues
# Even when make build shows SUCCESS

echo "=== Checking Build Logs for Issues ==="
echo

# Check for assertion failures
echo "--- Checking for assertion failures ---"
if grep -q "Assertion failed" logs/*.log 2>/dev/null; then
    echo "⚠️  ASSERTION FAILURES FOUND:"
    grep -n "Assertion failed" logs/*.log | head -10
else
    echo "✓ No assertion failures found"
fi
echo

# Check for segfaults or crashes
echo "--- Checking for crashes ---"
if grep -q -E "(Segmentation fault|Abort trap|core dumped)" logs/*.log 2>/dev/null; then
    echo "⚠️  CRASHES FOUND:"
    grep -n -E "(Segmentation fault|Abort trap|core dumped)" logs/*.log | head -10
else
    echo "✓ No crashes found"
fi
echo

# Check for test failures that might not be caught
echo "--- Checking for test failures ---"
# Look for actual test failure indicators, not just the word "fail" in test output
# Exclude "ERROR: Test error message" which is a legitimate test output pattern
if grep -E "(TEST FAILED|Test .* failed|ERROR: Test|FAILED:)" logs/run-tests.log logs/sanitize-tests.log logs/tsan-tests.log 2>/dev/null | grep -v "ERROR: Test error message" | grep -q .; then
    echo "⚠️  TEST FAILURES FOUND:"
    grep -n -E "(TEST FAILED|Test .* failed|ERROR: Test|FAILED:)" logs/run-tests.log logs/sanitize-tests.log logs/tsan-tests.log 2>/dev/null | grep -v "ERROR: Test error message" | head -10
else
    echo "✓ No test failures found"
fi
echo

# Check for memory errors from sanitizers
echo "--- Checking for memory errors ---"
if grep -q -E "(ERROR: AddressSanitizer|ERROR: LeakSanitizer|ERROR: UndefinedBehaviorSanitizer)" logs/sanitize-*.log 2>/dev/null; then
    echo "⚠️  MEMORY ERRORS FOUND:"
    grep -n -E "(ERROR: AddressSanitizer|ERROR: LeakSanitizer|ERROR: UndefinedBehaviorSanitizer)" logs/sanitize-*.log | head -10
else
    echo "✓ No memory errors found"
fi
echo

# Check for thread sanitizer issues
echo "--- Checking for thread safety issues ---"
if grep -q "WARNING: ThreadSanitizer" logs/tsan-*.log 2>/dev/null; then
    echo "⚠️  THREAD SAFETY ISSUES FOUND:"
    grep -n "WARNING: ThreadSanitizer" logs/tsan-*.log | head -10
else
    echo "✓ No thread safety issues found"
fi
echo

# Check for runtime errors and signals
echo "--- Checking for runtime errors ---"
if grep -q -E "(runtime error|SIGABRT|SIGSEGV|SIGBUS|SIGILL|SIGFPE)" logs/*.log 2>/dev/null; then
    echo "⚠️  RUNTIME ERRORS FOUND:"
    grep -n -E "(runtime error|SIGABRT|SIGSEGV|SIGBUS|SIGILL|SIGFPE)" logs/*.log | head -10
else
    echo "✓ No runtime errors found"
fi
echo

# Check for valgrind errors (if present)
echo "--- Checking for valgrind errors ---"
if grep -q -E "(Invalid read|Invalid write|Conditional jump|Uninitialised value)" logs/*.log 2>/dev/null; then
    echo "⚠️  VALGRIND ERRORS FOUND:"
    grep -n -E "(Invalid read|Invalid write|Conditional jump|Uninitialised value)" logs/*.log | head -10
else
    echo "✓ No valgrind errors found"
fi
echo

# Check for memory leaks from custom heap tracking
echo "--- Checking for memory leaks ---"
if grep -q -E "Warning: [0-9]+ memory leaks? detected" logs/*.log 2>/dev/null; then
    echo "⚠️  MEMORY LEAKS FOUND:"
    grep -n -E "Warning: [0-9]+ memory leaks? detected" logs/*.log | head -20
    echo
    echo "Note: These are from AgeRun's custom heap tracking, not AddressSanitizer."
    echo "Check the referenced memory_report_*.log files for details."
else
    echo "✓ No memory leaks detected"
fi
echo

# Summary with exit code
echo "=== Summary ==="
echo "Log files are in: logs/"
echo "To view a specific log: less logs/<logname>.log"
echo "To search logs: grep -r 'pattern' logs/"

# Exit with error if any issues found (note: memory leaks don't fail the build currently)
if grep -q -E "(Assertion failed|Segmentation fault|Abort trap|core dumped|ERROR: AddressSanitizer|ERROR: LeakSanitizer|ERROR: UndefinedBehaviorSanitizer|WARNING: ThreadSanitizer|runtime error|SIGABRT|SIGSEGV)" logs/*.log 2>/dev/null; then
    echo
    echo "⚠️  CRITICAL ISSUES DETECTED - Please review the logs above!"
    exit 1
else
    # Check if we have memory leaks (treat as error)
    if grep -q -E "Warning: [0-9]+ memory leaks? detected" logs/*.log 2>/dev/null; then
        echo
        echo "⚠️  MEMORY LEAKS DETECTED - These must be fixed!"
        exit 1
    else
        echo
        echo "✓ All checks passed - no issues detected"
        exit 0
    fi
fi
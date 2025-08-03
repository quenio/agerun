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
# Only check test logs, not exec logs (exec logs don't contain test results)
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

# Check for static analysis issues (scan-build)
echo "--- Checking for static analysis issues ---"
if grep -q -E "(scan-build: [1-9][0-9]* bug|warning:|error:|ERROR:|WARNING:)" logs/analyze-*.log 2>/dev/null | grep -v "No bugs found"; then
    echo "⚠️  STATIC ANALYSIS ISSUES FOUND:"
    grep -n -E "(scan-build: [1-9][0-9]* bug|warning:|error:|ERROR:|WARNING:)" logs/analyze-*.log | grep -v "No bugs found" | head -10
else
    echo "✓ No static analysis issues found"
fi
echo

# Check for compilation warnings and errors
echo "--- Checking for compilation warnings/errors ---"
# Look for compiler warnings and errors from gcc, clang, and zig
# Exclude known safe patterns from analysis logs
if grep -E "(warning:|error:|Error:|ERROR:)" logs/*.log 2>/dev/null | \
   grep -v "logs/analyze-" | \
   grep -v "No bugs found" | \
   grep -v "WARNING: .* memory leaks detected" | \
   grep -v "WARNING: ThreadSanitizer" | \
   grep -v "ERROR: Test" | \
   grep -v "ERROR: AddressSanitizer" | \
   grep -v "ERROR: LeakSanitizer" | \
   grep -v "ERROR: UndefinedBehaviorSanitizer" | \
   grep -v "ERROR: Failed to create execution frame" | \
   grep -v "ERROR: Unknown" | \
   grep -v "ERROR: Expected" | \
   grep -v "ERROR: Invalid" | \
   grep -v "ERROR: Assignment" | \
   grep -v "ERROR: Agent" | \
   grep -E "(\.(c|h|zig):[0-9]+:|warning:|error:)" | \
   grep -q .; then
    echo "⚠️  COMPILATION WARNINGS/ERRORS FOUND:"
    grep -n -E "(warning:|error:|Error:|ERROR:)" logs/*.log 2>/dev/null | \
        grep -v "logs/analyze-" | \
        grep -v "No bugs found" | \
        grep -v "WARNING: .* memory leaks detected" | \
        grep -v "WARNING: ThreadSanitizer" | \
        grep -v "ERROR: Test" | \
        grep -v "ERROR: AddressSanitizer" | \
        grep -v "ERROR: LeakSanitizer" | \
        grep -v "ERROR: UndefinedBehaviorSanitizer" | \
        grep -v "ERROR: Failed to create execution frame" | \
        grep -v "ERROR: Unknown" | \
        grep -v "ERROR: Expected" | \
        grep -v "ERROR: Invalid" | \
        grep -v "ERROR: Assignment" | \
        grep -v "ERROR: Agent" | \
        grep -E "(\.(c|h|zig):[0-9]+:|warning:|error:)" | \
        head -30
else
    echo "✓ No compilation warnings/errors found"
fi
echo

# Check for linker warnings and errors
echo "--- Checking for linker warnings/errors ---"
# Look for ld warnings, undefined symbols, duplicate symbols, etc.
if grep -E "(ld: warning:|ld: error:|undefined reference|undefined symbol|duplicate symbol|was built for newer.*version.*than being linked|relocation|cannot find -l)" logs/*.log 2>/dev/null | grep -q .; then
    echo "⚠️  LINKER WARNINGS/ERRORS FOUND:"
    grep -n -E "(ld: warning:|ld: error:|undefined reference|undefined symbol|duplicate symbol|was built for newer.*version.*than being linked|relocation|cannot find -l)" logs/*.log 2>/dev/null | head -30
else
    echo "✓ No linker warnings/errors found"
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

# Exit with error if any issues found
CRITICAL_ISSUES=0
WARNING_ISSUES=0

# Check for critical issues
if grep -q -E "(Assertion failed|Segmentation fault|Abort trap|core dumped|ERROR: AddressSanitizer|ERROR: LeakSanitizer|ERROR: UndefinedBehaviorSanitizer|WARNING: ThreadSanitizer|runtime error|SIGABRT|SIGSEGV)" logs/*.log 2>/dev/null; then
    CRITICAL_ISSUES=1
fi

# Check for memory leaks
if grep -q -E "Warning: [0-9]+ memory leaks? detected" logs/*.log 2>/dev/null; then
    CRITICAL_ISSUES=1
fi

# Check for compilation warnings/errors
if grep -E "(warning:|error:|Error:|ERROR:)" logs/*.log 2>/dev/null | \
   grep -v "logs/analyze-" | \
   grep -v "No bugs found" | \
   grep -v "WARNING: .* memory leaks detected" | \
   grep -v "WARNING: ThreadSanitizer" | \
   grep -v "ERROR: Test" | \
   grep -v "ERROR: AddressSanitizer" | \
   grep -v "ERROR: LeakSanitizer" | \
   grep -v "ERROR: UndefinedBehaviorSanitizer" | \
   grep -v "ERROR: Failed to create execution frame" | \
   grep -v "ERROR: Unknown" | \
   grep -v "ERROR: Expected" | \
   grep -v "ERROR: Invalid" | \
   grep -v "ERROR: Assignment" | \
   grep -v "ERROR: Agent" | \
   grep -E "(\.(c|h|zig):[0-9]+:|warning:|error:)" | \
   grep -q .; then
    WARNING_ISSUES=1
fi

# Check for linker warnings/errors
if grep -E "(ld: warning:|ld: error:|undefined reference|undefined symbol|duplicate symbol|was built for newer.*version.*than being linked|relocation|cannot find -l)" logs/*.log 2>/dev/null | grep -q .; then
    WARNING_ISSUES=1
fi

# Report results
if [ $CRITICAL_ISSUES -eq 1 ]; then
    echo
    echo "⚠️  CRITICAL ISSUES DETECTED - Please review the logs above!"
    exit 1
elif [ $WARNING_ISSUES -eq 1 ]; then
    echo
    echo "⚠️  WARNINGS DETECTED - Build succeeded but there are compilation/linking warnings to address!"
    exit 1
else
    echo
    echo "✓ All checks passed - no issues detected"
    exit 0
fi
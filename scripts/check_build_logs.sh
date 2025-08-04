#!/bin/bash

# Script to check build logs for potential issues
# Even when make build shows SUCCESS

echo "=== Checking Build Logs for Issues ==="
echo

# Track if we found any issues
FOUND_ISSUES=false

# Simple function to check if an error line matches whitelist patterns (before/error/after only)
is_whitelisted_error() {
    local log_file="$1"
    local line_num="$2"
    local error_line="$3"
    
    # Check if whitelist file exists
    if [ ! -f "error_whitelist.yaml" ]; then
        return 1  # No whitelist, report all errors
    fi
    
    # Get the lines before and after the error
    local before_line=""
    local after_line=""
    
    # Validate parsing worked (line_num should be a number)
    if ! [[ "$line_num" =~ ^[0-9]+$ ]]; then
        return 1  # Invalid parsing, report error
    fi
    
    if [ "$line_num" -gt 1 ]; then
        before_line=$(sed -n "$((line_num - 1))p" "$log_file" 2>/dev/null)
    fi
    after_line=$(sed -n "$((line_num + 1))p" "$log_file" 2>/dev/null)
    
    # Parse YAML and check for matches (ignore test and comment attributes)
    local in_intentional_errors=false
    local current_entry_error=""
    local current_entry_before=""
    local current_entry_after=""
    
    while IFS= read -r line; do
        # Skip comments and empty lines
        [[ "$line" =~ ^[[:space:]]*# ]] && continue
        [[ "$line" =~ ^[[:space:]]*$ ]] && continue
        
        # Check if we're in the intentional_errors section
        if [[ "$line" =~ ^intentional_errors: ]]; then
            in_intentional_errors=true
            continue
        fi
        
        # If we're not in the intentional_errors section, skip
        [ "$in_intentional_errors" = false ] && continue
        
        # Parse YAML entries (ignore test and comment)
        if [[ "$line" =~ ^[[:space:]]*-[[:space:]]*test: ]]; then
            # Reset for new entry
            current_entry_error=""
            current_entry_before=""
            current_entry_after=""
        elif [[ "$line" =~ ^[[:space:]]*error:[[:space:]]*\"(.*)\"$ ]]; then
            current_entry_error="${BASH_REMATCH[1]}"
        elif [[ "$line" =~ ^[[:space:]]*before:[[:space:]]*\"(.*)\"$ ]]; then
            current_entry_before="${BASH_REMATCH[1]}"
        elif [[ "$line" =~ ^[[:space:]]*after:[[:space:]]*\"(.*)\"$ ]]; then
            current_entry_after="${BASH_REMATCH[1]}"
        elif [[ "$line" =~ ^[[:space:]]*comment: ]] || [[ "$line" =~ ^[[:space:]]*-[[:space:]]*test: ]]; then
            # End of current entry - check if it matches
            if [ -n "$current_entry_error" ]; then
                # Check if all conditions match
                local error_match=false
                local before_match=false
                local after_match=false
                
                # Error must be contained in the error line
                if [[ "$error_line" == *"$current_entry_error"* ]]; then
                    error_match=true
                fi
                
                # Before line match (empty string means ignore)
                if [ -z "$current_entry_before" ]; then
                    before_match=true
                elif [[ "$before_line" == *"$current_entry_before"* ]]; then
                    before_match=true
                fi
                
                # After line match (empty string means ignore)
                if [ -z "$current_entry_after" ]; then
                    after_match=true
                elif [[ "$after_line" == *"$current_entry_after"* ]]; then
                    after_match=true
                fi
                
                # If all conditions match, this error is whitelisted
                if [ "$error_match" = true ] && [ "$before_match" = true ] && [ "$after_match" = true ]; then
                    return 0  # Whitelisted error
                fi
            fi
        fi
    done < "error_whitelist.yaml"
    
    # Check the last entry if file doesn't end with comment
    if [ -n "$current_entry_error" ]; then
        local error_match=false
        local before_match=false
        local after_match=false
        
        if [[ "$error_line" == *"$current_entry_error"* ]]; then
            error_match=true
        fi
        
        if [ -z "$current_entry_before" ]; then
            before_match=true
        elif [[ "$before_line" == *"$current_entry_before"* ]]; then
            before_match=true
        fi
        
        if [ -z "$current_entry_after" ]; then
            after_match=true
        elif [[ "$after_line" == *"$current_entry_after"* ]]; then
            after_match=true
        fi
        
        if [ "$error_match" = true ] && [ "$before_match" = true ] && [ "$after_match" = true ]; then
            return 0  # Whitelisted error
        fi
    fi
    
    return 1  # Not whitelisted
}


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
# Only look for actual compiler output patterns, not runtime errors
if grep -E "(\.(c|h|zig):[0-9]+:[0-9]+: (warning|error):|^(warning|error):)" logs/*.log 2>/dev/null | \
   grep -v "logs/analyze-" | \
   grep -q .; then
    echo "⚠️  COMPILATION WARNINGS/ERRORS FOUND:"
    grep -n -E "(\.(c|h|zig):[0-9]+:[0-9]+: (warning|error):|^(warning|error):)" logs/*.log 2>/dev/null | \
        grep -v "logs/analyze-" | \
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

# Check for deep copy support errors
echo "--- Checking for deep copy support errors ---"
if grep -q "no deep copy support" logs/*.log 2>/dev/null; then
    echo "⚠️  DEEP COPY SUPPORT ERRORS FOUND:"
    grep -n "no deep copy support" logs/*.log | head -10
    FOUND_ISSUES=true
else
    echo "✓ No deep copy support errors found"
fi
echo

# Check for method loading warnings
echo "--- Checking for method loading warnings ---"
if grep -q "Could not load methods from file" logs/*.log 2>/dev/null; then
    echo "⚠️  METHOD LOADING WARNINGS FOUND:"
    grep -n "Could not load methods from file" logs/*.log | head -10
    echo "Note: These warnings indicate the method store file doesn't exist yet."
    echo "This is expected during initial test runs but may indicate issues in production."
else
    echo "✓ No method loading warnings found"
fi
echo

# Check for unexpected method creation successes
echo "--- Checking for unexpected test behaviors ---"
if grep -q "WARNING: Method creation succeeded with invalid syntax (expected failure)" logs/*.log 2>/dev/null; then
    echo "⚠️  UNEXPECTED TEST BEHAVIOR FOUND:"
    grep -n "WARNING: Method creation succeeded with invalid syntax (expected failure)" logs/*.log | head -10
    echo "Note: A test expecting failure actually succeeded - this may indicate a validation bug."
    FOUND_ISSUES=true
else
    echo "✓ No unexpected test behaviors found"
fi
echo

# Check for method evaluation failures (excluding intentional test errors)
echo "--- Checking for method evaluation failures ---"
UNINTENTIONAL_ERRORS=""

# Get all the errors and check against whitelist
while IFS= read -r line; do
    # Parse the grep output: logs/filename.log:line_number:[timestamp] ERROR: ...
    # Extract file (everything before the first :line_number:)
    file=$(echo "$line" | sed 's/:\([0-9]\+\):.*$//')
    # Extract line number (the number after the first colon and before the second colon)
    line_num=$(echo "$line" | sed 's/^[^:]*:\([0-9]\+\):.*$/\1/')
    # Extract error text (everything after line_number:[timestamp])
    error_text=$(echo "$line" | sed 's/^[^:]*:[0-9]\+:\[.*\] //')
    
    # Check if this error is whitelisted
    if [ -n "$file" ] && [ -n "$line_num" ] && ! is_whitelisted_error "$file" "$line_num" "$error_text"; then
        if [ -z "$UNINTENTIONAL_ERRORS" ]; then
            UNINTENTIONAL_ERRORS="$line"
        else
            UNINTENTIONAL_ERRORS="$UNINTENTIONAL_ERRORS"$'\n'"$line"
        fi
    fi
done < <(grep -n "ERROR: Method evaluation failed" logs/*.log 2>/dev/null)

if [ -n "$UNINTENTIONAL_ERRORS" ]; then
    echo "⚠️  METHOD EVALUATION FAILURES FOUND:"
    echo "$UNINTENTIONAL_ERRORS" | head -10
    FOUND_ISSUES=true
else
    echo "✓ No unexpected method evaluation failures found"
fi
echo

# Check for missing AST errors
echo "--- Checking for missing AST errors ---"
UNINTENTIONAL_AST_ERRORS=""

# Get all AST errors and check against whitelist
while IFS= read -r line; do
    # Parse the grep output: logs/filename.log:line_number:[timestamp] ERROR: ...
    # Extract file (everything before the first :line_number:)
    file=$(echo "$line" | sed 's/:\([0-9]\+\):.*$//')
    # Extract line number (the number after the first colon and before the second colon)
    line_num=$(echo "$line" | sed 's/^[^:]*:\([0-9]\+\):.*$/\1/')
    # Extract error text (everything after line_number:[timestamp])
    error_text=$(echo "$line" | sed 's/^[^:]*:[0-9]\+:\[.*\] //')
    
    # Check against general whitelist first
    is_whitelisted=false
    if [ -n "$file" ] && [ -n "$line_num" ] && is_whitelisted_error "$file" "$line_num" "$error_text"; then
        is_whitelisted=true
    fi
    
    # Special handling for exec logs (expected AST errors)
    if echo "$line" | grep -q "logs/.*-exec\.log:"; then
        is_whitelisted=true
    fi
    
    # If not whitelisted, add to unintentional errors
    if [ "$is_whitelisted" = false ]; then
        if [ -z "$UNINTENTIONAL_AST_ERRORS" ]; then
            UNINTENTIONAL_AST_ERRORS="$line"
        else
            UNINTENTIONAL_AST_ERRORS="$UNINTENTIONAL_AST_ERRORS"$'\n'"$line"
        fi
    fi
done < <(grep -n "ERROR: Method has no AST" logs/*.log 2>/dev/null)

if [ -n "$UNINTENTIONAL_AST_ERRORS" ]; then
    echo "⚠️  MISSING AST ERRORS FOUND:"
    echo "$UNINTENTIONAL_AST_ERRORS" | head -10
    FOUND_ISSUES=true
else
    echo "✓ No missing AST errors found"
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

# Check for our new specific issues
if [ "$FOUND_ISSUES" = true ]; then
    CRITICAL_ISSUES=1
fi

# Check for compilation warnings/errors
if grep -E "(\.(c|h|zig):[0-9]+:[0-9]+: (warning|error):|^(warning|error):)" logs/*.log 2>/dev/null | \
   grep -v "logs/analyze-" | \
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
    echo "✓ All standard checks passed - performing deep analysis..."
    
    # Perform additional thorough analysis even when standard checks pass
    echo
    echo "=== Deep Log Analysis ==="
    echo
    
    DEEP_ISSUES_FOUND=false
    
    # Check for any ERROR: patterns we might have missed (with whitelist filtering)
    echo "--- Scanning for additional ERROR patterns ---"
    UNFILTERED_ERRORS=""
    
    # Get all ERROR patterns and filter through whitelist
    while IFS= read -r line; do
        # Skip known sanitizer errors
        if echo "$line" | grep -q -E "(ERROR: AddressSanitizer|ERROR: LeakSanitizer|ERROR: UndefinedBehaviorSanitizer|ERROR: ThreadSanitizer|ERROR: Test error message)"; then
            continue
        fi
        
        # Parse the grep output for whitelist checking
        file=$(echo "$line" | sed 's/:\([0-9]\+\):.*$//')
        line_num=$(echo "$line" | sed 's/^.*:\([0-9]\+\):.*$/\1/')
        error_text=$(echo "$line" | sed 's/^[^:]*:[0-9]\+://')
        
        # Check if this error is whitelisted
        if [ -n "$file" ] && [ -n "$line_num" ] && is_whitelisted_error "$file" "$line_num" "$error_text"; then
            continue  # Skip whitelisted errors
        fi
        
        # Add to unfiltered errors
        if [ -z "$UNFILTERED_ERRORS" ]; then
            UNFILTERED_ERRORS="$line"
        else
            UNFILTERED_ERRORS="$UNFILTERED_ERRORS"$'\n'"$line"
        fi
    done < <(grep -n -E "ERROR:|Error:" logs/*.log 2>/dev/null)
    
    if [ -n "$UNFILTERED_ERRORS" ]; then
        ERROR_COUNT=$(echo "$UNFILTERED_ERRORS" | wc -l)
        echo "⚠️  Found $ERROR_COUNT unwhitelisted ERROR messages in logs:"
        echo "$UNFILTERED_ERRORS" | head -10 | sed 's/^/    /'
        DEEP_ISSUES_FOUND=true
    else
        echo "✓ No additional unwhitelisted ERROR patterns found"
    fi
    echo
    
    # Check for any WARNING: patterns we might have missed
    echo "--- Scanning for additional WARNING patterns ---"
    WARNING_COUNT=$(grep -E "WARNING:|Warning:" logs/*.log 2>/dev/null | \
        grep -v "WARNING: .* memory leaks detected" | \
        grep -v "WARNING: ThreadSanitizer" | \
        grep -v "logs/analyze-" | \
        wc -l)
    
    if [ "$WARNING_COUNT" -gt 0 ]; then
        echo "⚠️  Found $WARNING_COUNT WARNING messages in logs:"
        grep -E "WARNING:|Warning:" logs/*.log 2>/dev/null | \
            grep -v "WARNING: .* memory leaks detected" | \
            grep -v "WARNING: ThreadSanitizer" | \
            grep -v "logs/analyze-" | \
            head -10 | sed 's/^/    /'
        DEEP_ISSUES_FOUND=true
    else
        echo "✓ No additional WARNING patterns found"
    fi
    echo
    
    # Check test output consistency
    echo "--- Checking test output consistency ---"
    TESTS_RUN=$(grep -c "^Running test:" logs/run-tests.log 2>/dev/null) || TESTS_RUN="0"
    TESTS_PASSED=$(grep -c "All .* tests passed" logs/run-tests.log 2>/dev/null) || TESTS_PASSED="0"
    
    if [ "$TESTS_RUN" -gt 0 ] && [ "$TESTS_PASSED" -eq 0 ]; then
        echo "⚠️  INCONSISTENCY: $TESTS_RUN tests ran but no 'All tests passed' messages found"
        DEEP_ISSUES_FOUND=true
    else
        echo "✓ Test output appears consistent ($TESTS_RUN tests, $TESTS_PASSED pass messages)"
    fi
    echo
    
    # Check for failed/error patterns in test names
    echo "--- Checking for suspicious test patterns ---"
    if grep -E "(FAILED|failed|FAIL|fail|ERROR|error)" logs/*.log 2>/dev/null | \
       grep -E "(test_|_test|Test)" | \
       grep -v "test.*failed.*passed" | \
       grep -v "expected.*fail" | \
       grep -v "ERROR: Test error message" | \
       grep -q .; then
        echo "⚠️  Found suspicious patterns in test-related output:"
        grep -E "(FAILED|failed|FAIL|fail|ERROR|error)" logs/*.log 2>/dev/null | \
            grep -E "(test_|_test|Test)" | \
            grep -v "test.*failed.*passed" | \
            grep -v "expected.*fail" | \
            grep -v "ERROR: Test error message" | \
            head -5 | sed 's/^/    /'
        DEEP_ISSUES_FOUND=true
    else
        echo "✓ No suspicious test patterns found"
    fi
    echo
    
    # Check for any "Could not" or "Cannot" messages
    echo "--- Checking for failure indicators ---"
    if grep -E "(Could not|Cannot|Unable to|Failed to)" logs/*.log 2>/dev/null | \
       grep -v "Could not load methods from file" | \
       grep -v "expected" | \
       grep -q .; then
        echo "⚠️  Found failure indicators:"
        grep -E "(Could not|Cannot|Unable to|Failed to)" logs/*.log 2>/dev/null | \
            grep -v "Could not load methods from file" | \
            grep -v "expected" | \
            head -10 | sed 's/^/    /'
        DEEP_ISSUES_FOUND=true
    else
        echo "✓ No unexpected failure indicators found"
    fi
    echo
    
    # Final deep analysis report
    echo "=== Deep Analysis Summary ==="
    if [ "$DEEP_ISSUES_FOUND" = true ]; then
        echo "⚠️  ADDITIONAL ISSUES FOUND during deep analysis!"
        echo "These patterns suggest potential problems that weren't caught by standard checks."
        echo "Please review the specific instances above and determine if they need attention."
        exit 1
    else
        echo "✓ Deep analysis complete - no additional issues detected"
        echo "The build logs appear clean."
        exit 0
    fi
fi
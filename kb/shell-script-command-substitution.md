# Shell Script Command Substitution Robustness

## Learning
Command substitution with error fallbacks can produce unexpected output that breaks subsequent operations. The pattern `$(command || echo "0")` can create double output when both the command and fallback execute, leading to errors like "integer expression expected" in bash comparisons.

## Importance
Shell scripts that process command output need robust error handling that doesn't introduce new failure modes. Improper command substitution can make scripts fragile and hard to debug.

## Example
```bash
# BAD: Can produce double output
TESTS_RUN=$(grep -c "^Running test:" logs/run-tests.log || echo "0")
# If grep outputs "0" AND the command succeeds, TESTS_RUN becomes "0 0"
# This breaks: [ $TESTS_RUN -gt 0 ] with "integer expression expected"

# GOOD: Proper variable assignment with error handling
TESTS_RUN=$(grep -c "^Running test:" logs/run-tests.log 2>/dev/null) || TESTS_RUN="0"
# Variable is set once, either by grep or by the fallback

# GOOD: Alternative using if statement
if TESTS_RUN=$(grep -c "^Running test:" logs/run-tests.log 2>/dev/null); then
    : # grep succeeded, TESTS_RUN is set
else
    TESTS_RUN="0"
fi
```

## Generalization
Safe command substitution patterns:
1. Use variable assignment with `||` after the substitution
2. Redirect stderr to avoid error messages in output
3. Use if statements for complex error handling
4. Test with both success and failure cases
5. Quote variables in comparisons to catch multi-word values

## Implementation
```bash
# Robust pattern for counting/numeric operations
safe_count() {
    local count
    count=$(grep -c "$1" "$2" 2>/dev/null) || count="0"
    echo "$count"
}

# Usage
FILE_COUNT=$(safe_count "pattern" "file.log")
if [ "$FILE_COUNT" -gt 0 ]; then
    echo "Found $FILE_COUNT matches"
fi

# For more complex operations
get_test_status() {
    local log_file=$1
    local passed failed
    
    passed=$(grep -c "PASSED" "$log_file" 2>/dev/null) || passed="0"
    failed=$(grep -c "FAILED" "$log_file" 2>/dev/null) || failed="0"
    
    echo "Passed: $passed, Failed: $failed"
}
```

## Related Patterns
- [Makefile POSIX Shell Compatibility](makefile-posix-shell-compatibility.md)
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Development Debug Tools](development-debug-tools.md)
- [Bash Command Parsing Patterns](bash-command-parsing-patterns.md)
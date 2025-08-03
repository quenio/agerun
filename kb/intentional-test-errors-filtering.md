# Filtering Intentional Test Errors in Build Scripts

## Learning
Test suites include intentional errors to verify error handling, but these are only intentional in specific test contexts. Build scripts must use context-aware filtering to distinguish between expected test errors and actual problems.

## Importance
Without context-aware filtering, intentional test errors create noise in build output. However, the same error messages could indicate real problems in production code, so filtering must be precise.

## Example
```bash
# Mark intentional errors in test output
printf("[INTENTIONAL ERROR] Testing invalid field access\n");
ar_data__get_map_data(string_value, "field");  // This will error

# Or use a prefix in error messages
ar_log__error(log, "[TEST EXPECTED] Cannot access field on non-map type");

# In check_build_logs.sh, filter these out:
grep -v "\[INTENTIONAL ERROR\]\|\[TEST EXPECTED\]" logs/run-tests.log | \
    grep "ERROR:" | \
    grep -v "known_test_patterns.txt"
```

## Generalization
Three strategies for handling intentional test errors:

1. **Marker Strategy**: Add prefixes like [TEST EXPECTED] to intentional errors
2. **Pattern File Strategy**: Maintain a file of known error patterns to exclude
3. **Context Strategy**: Check if errors occur within test functions (by name pattern)

## Implementation
```bash
# Context-aware filtering function
is_intentional_test_error() {
    local log_file=$1
    local line_num=$2
    local error_text=$3
    
    # Get context: 20 lines before the error
    local start=$((line_num - 20))
    local end=$((line_num + 5))
    [ $start -lt 1 ] && start=1
    
    local context=$(sed -n "${start},${end}p" "$log_file")
    
    # Check for specific test contexts:
    
    # 1. Test methods with special names
    if echo "$context" | grep -qE "__test_instruction_[0-9]+__|test_.*__.*Testing.*invalid"; then
        return 0
    fi
    
    # 2. Method evaluation errors in test contexts
    if echo "$error_text" | grep -q "Method evaluation failed" && \
       echo "$context" | grep -qE "__test_instruction_|ar_method_evaluator_tests|calculator_tests"; then
        return 0
    fi
    
    # 3. Missing AST errors for test methods
    if echo "$error_text" | grep -q "Method has no AST" && \
       echo "$context" | grep -qE "method '(echo|calc|exec_test_method)' version|ar_executable_tests"; then
        return 0
    fi
    
    # 4. General test runner context
    if echo "$context" | grep -qE "Running test:.*_tests|All.*tests passed"; then
        return 0
    fi
    
    return 1  # Not intentional
}

# Use in check_build_logs.sh
UNINTENTIONAL_ERRORS=""
while IFS= read -r line; do
    file=$(echo "$line" | cut -d: -f1)
    line_num=$(echo "$line" | cut -d: -f2)
    error_text=$(echo "$line" | cut -d: -f3-)
    
    if ! is_intentional_test_error "$file" "$line_num" "$error_text"; then
        UNINTENTIONAL_ERRORS="${UNINTENTIONAL_ERRORS}${line}\n"
    fi
done < <(grep -n "ERROR: Method evaluation failed" logs/*.log 2>/dev/null)
```

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Development Debug Tools](development-debug-tools.md)
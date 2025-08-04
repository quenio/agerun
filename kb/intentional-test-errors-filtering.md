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

# In check_logs.py, filter these out using whitelist:
# See check_logs.py for implementation with:
# - Pattern-based whitelist
# - Context-aware filtering
# - Configurable error patterns
```

## Generalization
Three strategies for handling intentional test errors:

1. **Marker Strategy**: Add prefixes like [TEST EXPECTED] to intentional errors
2. **Pattern File Strategy**: Maintain a file of known error patterns to exclude
3. **Context Strategy**: Check if errors occur within test functions (by name pattern)

## Implementation
The Python implementation in `check_logs.py` uses a comprehensive whitelist approach:

```python
# Example whitelist patterns from check_logs.py
ERROR_WHITELIST = [
    # Test-specific errors
    "ERROR: Method evaluation failed.*__test_instruction_",
    "ERROR: Method has no AST.*exec_test_method",
    "ERROR: Invalid method name.*Testing.*invalid",
    
    # Context-specific patterns
    "ERROR:.*\\[TEST EXPECTED\\]",
    "ERROR:.*\\[INTENTIONAL ERROR\\]",
    
    # Specific test files
    "ar_method_evaluator_tests.*Method evaluation failed",
    "calculator_tests.*Invalid expression",
]

def is_whitelisted_error(line, context_lines):
    """Check if an error line matches whitelist patterns"""
    for pattern in ERROR_WHITELIST:
        if re.search(pattern, line):
            return True
    
    # Check context for test indicators
    test_indicators = [
        "Running test:",
        "All.*tests passed",
        "__test_instruction_",
    ]
    
    for indicator in test_indicators:
        for ctx_line in context_lines:
            if indicator in ctx_line:
                return True
    
    return False
```

Key advantages of the Python implementation:
- **Maintainable whitelist**: All patterns in one place
- **Regex support**: More flexible pattern matching
- **Context analysis**: Can examine surrounding lines
- **Easy updates**: Add new patterns without modifying logic
- **Better reporting**: Can show which pattern matched

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Development Debug Tools](development-debug-tools.md)
# Test String Selection Strategy

## Learning
Test strings should be clearly synthetic and distinguishable from real system messages or data to avoid confusion during debugging and test output analysis.

## Importance
Using test strings that resemble actual system messages (like "__wake__", "__sleep__", "error", "warning") can cause:
- Confusion when reading test output
- False pattern matches in log analysis
- Difficulty distinguishing intentional test data from real system behavior
- Misleading grep results when searching for actual system issues

## Example
```c
// BAD: Test string resembles system message
void test_type_mismatch_error(void) {
    // Using "__wake__" could be confused with actual wake messages
    ar_data_t *own_message = ar_data__create_string("__wake__");  // BAD: Looks like system message
    // Test expects error with this string
    assert(strstr(error_msg, "__wake__") != NULL);
}

// GOOD: Clearly synthetic test string  
void test_type_mismatch_error(void) {
    // Using clearly synthetic test data
    ar_data_t *own_message = ar_data__create_string("test_string_value");
    // Test expects error with this string
    assert(strstr(error_msg, "test_string_value") != NULL);
}

// GOOD: Descriptive test-specific strings
ar_data_t *own_test_data = ar_data__create_string("test_validation_input_42");
ar_data_t *own_test_msg = ar_data__create_string("synthetic_test_message_001");
ar_data_t *own_test_err = ar_data__create_string("intentional_test_error_case");
```

## Generalization
When selecting test strings:
1. **Avoid system-like patterns**: Don't use "__prefix__", "error", "warning", "init", "shutdown"
2. **Use clear test markers**: Include "test", "synthetic", "dummy", "mock" in the string
3. **Be descriptive**: "test_string_value" is better than just "test"
4. **Add uniqueness**: Include test case numbers or specific identifiers when needed
5. **Consider grep-ability**: Make strings easy to find and distinguish from production strings

## Implementation
Best practices for test string selection:
```c
// Pattern: test_<purpose>_<identifier>
ar_data_t *own_input = ar_data__create_string("test_parser_input_001");
ar_data_t *own_output = ar_data__create_string("test_evaluator_output_xyz");

// Pattern: synthetic_<type>_<case>  
ar_data_t *own_msg = ar_data__create_string("synthetic_message_case_42");

// Pattern: dummy_<component>_<value>
ar_data_t *own_data = ar_data__create_string("dummy_agent_name_123");
```

## Related Patterns
- [Test Error Marking Strategy](test-error-marking-strategy.md)
- [Whitelist Specificity Pattern](whitelist-specificity-pattern.md)
- [Intentional Test Errors Filtering](intentional-test-errors-filtering.md)
# Error Detection Validation Testing

## Learning
Error suppression systems (like whitelists) must be tested by temporarily introducing errors that should NOT be suppressed. This validates that detection systems remain effective and don't become too broad.

## Importance
Without validation testing, error suppression can silently grow to mask real problems. Regular validation ensures safety nets work correctly and catch unintended errors.

## Example
Testing whitelist effectiveness:
```c
// Step 1: Verify intentional error is whitelisted
static void test_intentional_error_whitelisted(void) {
    // This error should be in whitelist
    const char *path[] = {"type_mismatch_test_field"};
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("message", path, 1);
    ar_data_t *own_wake = ar_data__create_string("__wake__");
    ar_frame_t *own_frame = ar_frame__create(memory, context, own_wake);
    
    // Run test - error should be suppressed
    ar_data_t *result = ar_expression_evaluator__evaluate(evaluator, own_frame, own_ast);
    // Build should pass despite error
}

// Step 2: Temporarily test that similar errors are caught
static void test_similar_error_not_whitelisted(void) {
    // TEMPORARY TEST: Different field name
    const char *path[] = {"sender"};  // NOT in whitelist
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("message", path, 1);
    ar_data_t *own_wake = ar_data__create_string("__wake__");
    ar_frame_t *own_frame = ar_frame__create(memory, context, own_wake);
    
    // This should produce unwhitelisted error
    ar_data_t *bad_result = ar_expression_evaluator__evaluate(evaluator, own_frame, own_ast);
    (void)bad_result; // Suppress unused warning
    
    // Run check-logs - should report this error
}
```

Validation results:
```bash
# Running check-logs catches the simulated error
=== Deep Log Analysis ===
--- Scanning for additional ERROR patterns ---
⚠️  Found 3 unwhitelisted ERROR messages in logs:
    logs/run-tests.log:939:ERROR: Cannot access field 'sender' on STRING value "__wake__"
```

## Generalization
Validation testing strategies:

1. **Simulation testing**: Temporarily add errors that should be caught
2. **Boundary testing**: Test variations just outside whitelist patterns
3. **Regular audits**: Periodically validate all suppression rules
4. **Deep analysis verification**: Ensure multi-layer detection works
5. **Removal testing**: Remove after verification to avoid test pollution

## Implementation
Systematic validation process:

```c
// 1. Create validation test framework
typedef struct {
    const char* description;
    const char* error_pattern;
    bool should_be_whitelisted;
} validation_test_t;  // EXAMPLE: Hypothetical type for validation framework

validation_test_t validation_tests[] = {  // EXAMPLE: Using hypothetical type
    {"Intentional error IS whitelisted", "type_mismatch_test_field", true},
    {"Similar error NOT whitelisted", "sender", false},
    {"Generic field NOT whitelisted", "method_name", false},
    {"Production field NOT whitelisted", "data", false}
};

// 2. Run validation suite
void validate_error_detection(void) {  // EXAMPLE: Hypothetical validation function
    for (size_t i = 0; i < sizeof(validation_tests)/sizeof(validation_test_t); i++) {  // EXAMPLE: Using hypothetical type
        validation_test_t* test = &validation_tests[i];  // EXAMPLE: Using hypothetical type
        
        // Generate the error
        const char *path[] = {test->error_pattern};
        ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("message", path, 1);
        // ... trigger error ...
        
        // Check if properly handled
        bool was_whitelisted = check_if_whitelisted(test->error_pattern);  // EXAMPLE: Hypothetical function
        assert(was_whitelisted == test->should_be_whitelisted);
    }
}

// 3. Clean up after validation
// Remove any temporary test code that generates unwanted errors
```

Best practices:
1. Run validation tests in CI/CD periodically
2. Document why each pattern should/shouldn't be whitelisted
3. Use descriptive test names indicating validation purpose
4. Clean up temporary code immediately after validation
5. Track whitelist size over time as a metric

## Related Patterns
- [Whitelist Specificity Pattern](whitelist-specificity-pattern.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Build Log Verification Requirement](build-log-verification-requirement.md)
- [Deep Analysis Safety Nets](comprehensive-output-review.md)
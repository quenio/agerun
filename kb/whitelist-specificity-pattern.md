# Whitelist Specificity Pattern

## Learning
Whitelist entries for intentional test errors must use unique identifiers to prevent accidentally masking real bugs. Generic field names or error patterns can hide actual problems when code evolves.

## Importance
As codebases grow, whitelists can become overly broad. If a whitelist entry matches both intentional test errors and real bugs, critical issues may go unnoticed. Using unique identifiers ensures only the specific intentional error is ignored.

## Example
Making an intentional test error specific:
```c
// BAD: Generic field name that might appear in real code
static void test_evaluate_type_mismatch_error_message(void) {
    // Create wake message test scenario
    ar_data_t *own_message = ar_data__create_string("__wake__");
    ar_frame_t *own_frame = ar_frame__create(mut_memory, own_context, own_message);
    
    // Generic field name could mask real errors
    const char *path[] = {"method_name"};  // BAD: Too generic
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("message", path, 1);
}

// GOOD: Unique field name that won't appear elsewhere
static void test_evaluate_type_mismatch_error_message(void) {
    // Create wake message test scenario
    ar_data_t *own_message = ar_data__create_string("__wake__");
    ar_frame_t *own_frame = ar_frame__create(mut_memory, own_context, own_message);
    
    // Unique field name prevents masking real errors
    const char *path[] = {"type_mismatch_test_field"};  // GOOD: Unique to this test
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("message", path, 1);
}
```

Corresponding whitelist becomes specific:
```yaml
# BAD: Could mask real errors accessing message.method_name
- context: "ar_expression_evaluator_tests"
  message: "ERROR: Cannot access field 'method_name' on STRING value \"__wake__\""
  comment: "Intentional test error"

# GOOD: Only matches this specific test
- context: "ar_expression_evaluator_tests"
  message: "ERROR: Cannot access field 'type_mismatch_test_field' on STRING value \"__wake__\""
  comment: "Intentional test error - unique field prevents masking real errors"
```

## Generalization
Strategies for whitelist specificity:

1. **Unique identifiers**: Use field names like `test_only_field`, `intentional_error_marker`
2. **Test-specific values**: Include test name in error strings
3. **Descriptive names**: `type_mismatch_test_field` clearly indicates test purpose
4. **Validation testing**: Simulate errors that should NOT be whitelisted
5. **Regular review**: Audit whitelists for overly broad patterns

## Implementation
Process for implementing specific whitelists:

```c
// 1. Choose unique identifiers for test errors
#define TEST_FIELD_NAME "intentional_test_field_12345"

// 2. Use in test code
static void test_field_access_error(void) {
    ar_data_t *own_string = ar_data__create_string("test");
    const char *path[] = {TEST_FIELD_NAME};
    ar_expression_ast_t *own_ast = ar_expression_ast__create_memory_access("data", path, 1);
    
    // This will produce: ERROR: Cannot access field 'intentional_test_field_12345'...
    ar_data_t *result = ar_expression_evaluator__evaluate(evaluator, frame, own_ast);
    assert(result == NULL);
}

// 3. Validate with simulation testing
static void verify_whitelist_specificity(void) {
    // Temporarily add code that should NOT be whitelisted
    const char *real_path[] = {"sender"};  // Common field name
    ar_expression_ast_t *own_bad_ast = ar_expression_ast__create_memory_access("message", real_path, 1);
    
    // Run this and verify it's caught as an error
    // Remove after verification
}
```

Verification checklist:
1. Run tests with specific whitelist entries
2. Temporarily introduce similar but different errors
3. Verify new errors are caught by check-logs
4. Document unique identifier choice in test comments

## Related Patterns
- [Test Error Marking Strategy](test-error-marking-strategy.md)
- [Intentional Test Errors Filtering](intentional-test-errors-filtering.md)
- [Systematic Error Whitelist Reduction](systematic-error-whitelist-reduction.md)
- [Error Detection Validation Testing](error-detection-validation-testing.md)
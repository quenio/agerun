# Test Error Marking Strategy

## Learning
Tests that intentionally trigger errors should clearly mark where they expect errors to occur. This makes it easier to distinguish intentional errors from real bugs in build output and enables more effective error filtering in CI/CD pipelines.

## Importance
Without clear marking, intentional test errors create noise in build logs and can mask real problems. Proper marking enables automated tools to filter expected errors while catching unexpected ones.

## Example
```c
// GOOD: Clear markers for intentional errors
static void test_invalid_syntax_handling(void) {
    printf("[TEST EXPECTS ERROR] Testing invalid assignment operator\n");
    
    // This will intentionally fail
    ar_method_t* method = ar_method__create("test", "x = 1", "1.0.0");  // BAD: Invalid syntax
    
    printf("[END EXPECTED ERROR]\n");
    
    // Verify it failed as expected
    assert(method == NULL || ar_method__get_ast(method) == NULL);
}

// GOOD: Using test name patterns
static void test_method_evaluator__invalid_field_access(void) {
    // Test name clearly indicates this tests invalid behavior
    ar_data_t* string_data = ar_data__create_string("hello");
    
    // Intentional error - accessing field on non-map
    ar_log__error(log, "[INTENTIONAL] Testing field access on string");
    ar_data_t* result = ar_data__get_map_data(string_data, "field");
    assert(result == NULL);
    
    ar_data__destroy(string_data);
}

// GOOD: Section marking for multiple errors
static void test_error_recovery(void) {
    printf("=== BEGIN INTENTIONAL ERROR SECTION ===\n");
    
    // Multiple operations that should fail
    test_null_pointer_handling();
    test_invalid_type_conversion();
    test_out_of_bounds_access();
    
    printf("=== END INTENTIONAL ERROR SECTION ===\n");
}
```

## Generalization
Error marking strategies:
1. **Prefix markers**: `[INTENTIONAL ERROR]`, `[TEST EXPECTED]`, `[EXPECTED FAILURE]`
2. **Section markers**: Clear BEGIN/END blocks around error-prone code
3. **Test naming**: Include "invalid", "error", "failure" in test names
4. **Log context**: Add context before triggering errors
5. **Assertion comments**: Document why errors are expected

## Implementation
```c
// Macro for marking expected errors
#define BEGIN_EXPECTED_ERRORS() \
    printf("[BEGIN EXPECTED ERRORS] %s:%d\n", __FILE__, __LINE__)

#define END_EXPECTED_ERRORS() \
    printf("[END EXPECTED ERRORS] %s:%d\n", __FILE__, __LINE__)

// Usage in tests
void test_parser_error_handling(void) {
    BEGIN_EXPECTED_ERRORS();
    
    // Test various parse errors
    ar_method_parser_t* parser = ar_method_parser__create(NULL);
    ar_method_ast_t* ast = ar_method_parser__parse(parser, "invalid := syntax :=");
    assert(ast == NULL);
    
    END_EXPECTED_ERRORS();
    
    ar_method_parser__destroy(parser);
}

// Build script filtering
#!/bin/bash
# Filter out marked errors from logs
grep -v "\[INTENTIONAL ERROR\]\|\[TEST EXPECTED\]\|\[BEGIN EXPECTED ERRORS\]" logs/test.log | \
    grep "ERROR:" > actual_errors.log

if [ -s actual_errors.log ]; then
    echo "Found unexpected errors:"
    cat actual_errors.log
    exit 1
fi
```

## Related Patterns
- [Intentional Test Errors Filtering](intentional-test-errors-filtering.md)
- [BDD Test Structure](bdd-test-structure.md)
- [Test Diagnostic Output Preservation](test-diagnostic-output-preservation.md)
- [Whitelist Simplification Pattern](whitelist-simplification-pattern.md)
- [Uniform Filtering Application](uniform-filtering-application.md)
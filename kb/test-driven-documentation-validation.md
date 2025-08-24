# Test-Driven Documentation Validation

## Learning
Writing tests for new functionality, especially error conditions, serves as a validation mechanism for documentation accuracy. When tests pass, they confirm that documented behavior matches actual implementation, preventing documentation drift.

## Importance
Documentation often becomes outdated as code evolves. By writing tests that exercise documented behavior, we create a living validation that ensures documentation remains accurate. This is especially valuable for error messages and edge cases that might otherwise go untested.

## Example
```c
// Documentation claims: "Logs 'NULL filename provided to YAML reader' on NULL filename"
// Test validates this claim:
TEST(yaml_reader, logs_error_for_null_filename) {
    ar_yaml_reader_t *own_reader = ar_yaml_reader__create(ref_log);
    
    // Trigger the documented error condition
    ar_data_t *result = ar_yaml_reader__read_from_file(own_reader, NULL);
    AR_ASSERT_NULL(result);
    
    // If test passes and we see the error in output, documentation is accurate
    // If error message differs, we catch the discrepancy immediately
    
    ar_yaml_reader__destroy(own_reader);
}

// Documentation update triggered by test:
// Before test: "Returns NULL on invalid input"  // Vague
// After test: "Returns NULL and logs 'NULL filename provided to YAML reader'"  // Precise
```

## Generalization
**What to validate through tests**:
1. **Error messages**: Exact text of error conditions
2. **Return values**: What functions return in each scenario
3. **State changes**: How operations affect object state
4. **Side effects**: Files created, messages logged, resources allocated
5. **Edge cases**: Boundary conditions and special values

**Documentation validation workflow**:
1. Write documentation describing intended behavior
2. Write test that exercises that exact behavior
3. Run test - if it fails, either:
   - Fix implementation to match documentation, OR
   - Update documentation to match reality
4. Keep test as permanent validation

**Red flags indicating documentation drift**:
- Tests pass but behavior differs from docs
- Error messages in tests don't match documented messages
- Parameter counts or types differ
- Return values don't match documentation

## Implementation
```c
// Pattern: Test name matches documentation section
// Documentation: "### Error Handling"
// Test: test_module__error_handling

// Documentation says: "Logs error and returns -1 on failure"
TEST(module, error_handling_matches_documentation) {
    // Setup error condition
    ar_data_t *own_module = ar_data__create_map();
    
    // Trigger documented behavior
    int result = perform_invalid_operation(own_module);
    
    // Validate ALL documented claims
    AR_ASSERT_EQ(result, -1);  // "returns -1"
    // Check test output for error message  // "Logs error"
    
    ar_data__destroy(own_module);
}

// Pattern: Test for each documented error message
static void validate_error_messages(void) {
    // From docs: "Error: Failed to open file"
    test_file_open_error();
    
    // From docs: "Error: Invalid parameter"  
    test_invalid_parameter_error();
    
    // From docs: "Error: Out of memory"
    test_memory_error();
}
```

## Benefits
- **Living documentation**: Tests ensure docs stay current
- **Catch discrepancies early**: Before users encounter them
- **Improve documentation quality**: Forces precise, testable claims
- **Reduce debugging time**: Accurate error messages help users

## Test-Documentation Synchronization
```markdown
<!-- In module.md -->
## Error Handling

The module reports the following errors:
- `NULL data provided to writer` - when data parameter is NULL
- `NULL filename provided to writer` - when filename parameter is NULL  
- `Failed to open file for writing: <filename>` - when file cannot be opened

<!-- Corresponding tests must exist for each error -->
```

```c
// In module_tests.c
TEST(module, error_null_data) { /* validates first error */ }
TEST(module, error_null_filename) { /* validates second error */ }
TEST(module, error_file_open) { /* validates third error */ }
```

## Related Patterns
- [Test-First Verification Practice](test-first-verification-practice.md)
- [Documentation Standards Integration](documentation-standards-integration.md)
- [Test Completeness Enumeration](test-completeness-enumeration.md)
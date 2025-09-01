# Test Complexity as Code Smell

## Learning
Complex test setup with loops, conditions, and retry logic indicates testing at the wrong abstraction level. Tests should be simple and obvious. If you need complex logic to set up a test scenario, you're likely fighting the framework rather than testing behavior.

## Importance
Complex tests are:
- Hard to understand and maintain
- Fragile and prone to breaking
- Often testing implementation details rather than behavior
- A sign of poor test design or wrong abstraction level
- Difficult to debug when they fail

## Example
```c
// CODE SMELL: Complex test with loops and conditions
static void test_malloc_failure(void) {
    bool found_failure = false;
    for (int tries = 1; tries <= 5 && !found_failure; tries++) {
        reset_counters();
        for (int consecutive = 1; consecutive <= tries; consecutive++) {
            g_fail_at_malloc = consecutive;
            ar_assignment_instruction_parser_t* parser = 
                ar_assignment_instruction_parser__create(NULL);
            if (!parser) {
                found_failure = true;
                break;
            }
        }
    }
    AR_ASSERT(found_failure, "Should eventually fail");
}

// CLEAN: Simple test with direct control
static void test_parser_creation_failure(void) {
    // Given: Parser creation will fail
    should_fail_parser_create = true;
    
    // When: Creating parser
    ar_log_t* log = ar_log__create();
    ar_assignment_instruction_parser_t* parser = 
        ar_assignment_instruction_parser__create(log);
    
    // Then: Returns NULL and logs error
    AR_ASSERT(parser == NULL, "Parser creation should fail");
    AR_ASSERT(ar_log__get_last_error_message(log) != NULL, 
             "Error should be logged");
    
    ar_log__destroy(log);
}
```

## Generalization
Signs of test complexity smell:
- Nested loops in test setup
- Multiple conditional branches
- Retry logic or attempts
- Complex state management
- Unclear what's being tested
- Need for extensive comments to explain test logic

Solutions:
- Mock at higher abstraction level
- Use simple boolean flags for control
- Test one behavior per test
- Make test intent obvious from code structure

## Implementation
```c
// Pattern for simple test structure
void test_specific_behavior(void) {
    // Given: Clear setup (1-3 lines)
    ar_data_t* data = ar_data__create_integer(42);
    
    // When: Single action being tested
    ar_data_t* copy = ar_data__shallow_copy(data);
    
    // Then: Clear assertions
    AR_ASSERT(copy != NULL, "Copy should succeed");
    AR_ASSERT(ar_data__get_integer(copy) == 42, "Value should match");
    
    // Cleanup
    ar_data__destroy(data);
    ar_data__destroy(copy);
}
```

## Related Patterns
- [Mock at Right Level Pattern](mock-at-right-level-pattern.md)
- [Code Smell Quick Detection](code-smell-quick-detection.md)
- [Test Effectiveness Verification](test-effectiveness-verification.md)
- [Test Complexity Reduction Pattern](test-complexity-reduction-pattern.md)
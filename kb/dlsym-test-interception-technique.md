# DLSym Test Interception Technique

## Learning
The dlsym function interception technique allows testing error conditions that cannot be naturally triggered, such as memory allocation failures or resource creation failures. By intercepting function calls at runtime, tests can simulate failures at specific points to verify error handling and cleanup behavior.

## Importance
Many error conditions are difficult or impossible to test naturally:
- Memory allocation failures occur rarely in test environments
- Resource creation failures may depend on system state
- Testing cleanup behavior requires controlled failure scenarios
- Without these tests, error handling code remains unverified

## Implementation Pattern

### 1. Mock Implementation with Control Variables
```c
// Control which creation should fail
static int fail_at_parser = -1;
static int current_parser = 0;

// Mock the specialized parser creation function
ar_assignment_instruction_parser_t* ar_assignment_instruction_parser__create(ar_log_t* log) {
    (void)log;
    current_parser++;
    if (current_parser == fail_at_parser) {
        printf("  Mock: Failing assignment parser creation (#%d)\n", current_parser);
        return NULL;
    }
    printf("  Mock: Created assignment parser (#%d)\n", current_parser);
    return (ar_assignment_instruction_parser_t*)0x1000; // Fake pointer
}
```

### 2. Memory Function Wrappers for dlsym
```c
void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn real_malloc = NULL;
    if (!real_malloc) {
        union { void* obj; malloc_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "malloc");
        real_malloc = converter.func;
    }
    return real_malloc(size);
}
```

### 3. Test Helper Functions
```c
static void run_test(const char* test_name, int fail_at, const char* expected_error) {
    reset_counters();
    fail_at_parser = fail_at;
    
    ar_log_t* log = ar_log__create();
    ar_instruction_parser_t* parser = ar_instruction_parser__create(log);
    
    if (!parser) {
        const char* error_msg = ar_log__get_last_error_message(log);
        assert(error_msg && strstr(error_msg, expected_error));
    }
    
    ar_log__destroy(log);
}
```

## Advantages Over Weak Tests

### Weak Test Pattern (Limited Verification)
```c
// Only verifies function can be called - doesn't test failure paths
static void test_instruction_parser__create_succeeds(void) {
    ar_log_t* log = ar_log__create();
    ar_instruction_parser_t* parser = ar_instruction_parser__create(log);
    assert(parser != NULL);
    ar_instruction_parser__destroy(parser);
    ar_log__destroy(log);
}
```

### Strong Test Pattern (Complete Verification)
```c
// Verifies specific error condition and cleanup behavior
static void test_instruction_parser__logs_assignment_parser_failure(void) {
    run_test("Fail at assignment parser (#1)", 1, 
             "Failed to create assignment instruction parser");
    
    // Verify cleanup: parsers_created should equal parsers_destroyed
    assert(parsers_created == parsers_destroyed);
}
```

## Integration with Build System

### Makefile Exclusion for Sanitizers
```makefile
# Filter out dlsym tests from sanitizer builds
SANITIZER_TEST_SRC = $(filter-out %_dlsym_tests.c,$(TEST_SRC))

# Regular tests include all
run-tests: $(ALL_TEST_BIN)
    @for test in $(ALL_TEST_BIN_NAMES); do ./$$test; done

# Sanitizer tests exclude dlsym
sanitize-tests: $(SANITIZER_TEST_BIN)
    @for test in $(SANITIZER_TEST_BIN_NAMES); do ./$$test; done
```

## Best Practices

1. **Naming Convention**: Use `*_dlsym_tests.c` suffix for dlsym tests
2. **Documentation**: Add clear comments explaining the exclusion
3. **Leak Tracking**: Count created/destroyed resources to verify cleanup
4. **Systematic Testing**: Test each failure point individually
5. **Output Clarity**: Use descriptive printf statements for debugging

## Example Test Output
```
=== Fail at assignment parser (#1) ===
  Mock: Failing assignment parser creation (#1)
Result: FAILED - parser is NULL (expected)
✓ Error logged correctly: Failed to create assignment instruction parser

Cleanup tracking:
  Parsers: created=0, destroyed=0
  OK: All parsers properly cleaned up
```

## Related Patterns
- [Sanitizer Test Exclusion Pattern](sanitizer-test-exclusion-pattern.md)
- [Test Effectiveness Verification](test-effectiveness-verification.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
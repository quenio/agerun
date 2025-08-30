# Test Assertion Strength Patterns

## Learning
Test assertions vary in strength from weak (minimal verification) to strong (comprehensive validation). Strong tests verify specific behavior, error messages, and side effects, while weak tests only confirm basic success/failure without validating the actual behavior.

## Importance
Weak tests provide false confidence:
- They pass even when implementation is broken
- They don't catch regressions
- They don't verify error handling quality
- They leave error paths untested
- They don't ensure proper cleanup

## Weak Test Patterns (Avoid These)

### Pattern 1: Success-Only Verification
```c
// WEAK: Only tests happy path, no error verification
static void test_parser__create_succeeds(void) {
    ar_log_t* log = ar_log__create();
    ar_parser_t* parser = ar_parser__create(log);
    assert(parser != NULL);  // Minimal assertion
    ar_parser__destroy(parser);
    ar_log__destroy(log);
}
```

### Pattern 2: Non-Specific Error Checking
```c
// WEAK: Knows it failed but not why
static void test_parser__fails_somehow(void) {
    ar_parser_t* parser = ar_parser__parse(NULL, "invalid");
    assert(parser == NULL);  // Doesn't verify error message
}
```

### Pattern 3: Incomplete State Verification
```c
// WEAK: Doesn't verify all affected state
static void test_data__update(void) {
    ar_data_t* data = ar_data__create_int(42);
    ar_data__update(data, 100);
    assert(ar_data__get_int(data) == 100);
    // Doesn't check: ownership, type, error state
    ar_data__destroy(data);
}
```

## Strong Test Patterns (Use These)

### Pattern 1: Specific Error Message Verification
```c
// STRONG: Verifies exact error and location
static void test_parser__logs_null_input_error(void) {
    ar_log_t* log = ar_log__create();
    ar_parser_t* parser = ar_parser__create(log);
    
    ar_instruction_ast_t* ast = ar_parser__parse(parser, NULL);
    assert(ast == NULL);
    
    // Verify specific error message
    const char* error = ar_log__get_last_error_message(log);
    assert(error != NULL);
    assert(strstr(error, "NULL instruction") != NULL);
    
    // Verify error position if applicable
    assert(ar_log__get_last_error_position(log) == 0);
    
    ar_parser__destroy(parser);
    ar_log__destroy(log);
}
```

### Pattern 2: Cleanup Verification
```c
// STRONG: Verifies proper resource cleanup on failure
static void test_parser__cleans_up_on_partial_failure(void) {
    // Track allocations
    int initial_allocations = get_allocation_count();
    
    // Simulate failure at specific point
    simulate_failure_at_allocation(5);
    
    ar_parser_t* parser = ar_parser__create(NULL);
    assert(parser == NULL);  // Expected failure
    
    // Verify no leaks despite failure
    assert(get_allocation_count() == initial_allocations);
}
```

### Pattern 3: Behavior Chain Verification
```c
// STRONG: Verifies complete behavior sequence
static void test_agent__message_handling(void) {
    ar_system_t* system = ar_system__create();
    ar_agent_t* agent = ar_agent__create(system, "test", "1.0.0");
    
    // Send message
    ar_message_t* msg = ar_message__create("test", NULL);
    ar_agent__send(agent, msg);
    
    // Verify message queued
    assert(ar_agent__has_pending_messages(agent));
    
    // Process message
    ar_system__process_next_message(system);
    
    // Verify message processed
    assert(!ar_agent__has_pending_messages(agent));
    
    // Verify side effects
    assert(ar_agent__get_message_count(agent) == 1);
    
    ar_agent__destroy(agent);
    ar_system__destroy(system);
}
```

## Transformation Strategies

### From Weak to Strong: Add Specific Assertions
```c
// BEFORE (Weak)
assert(result != NULL);

// AFTER (Strong)
assert(result != NULL);
assert(strcmp(result->type, "expected_type") == 0);
assert(result->value == expected_value);
assert(result->error == NULL);
```

### From Weak to Strong: Verify Error Details
```c
// BEFORE (Weak)
assert(parse_failed);

// AFTER (Strong)
assert(parse_failed);
assert(strstr(error_msg, "Expected '(' at position 5") != NULL);
assert(error_position == 5);
assert(error_count == 1);
```

### From Weak to Strong: Check Side Effects
```c
// BEFORE (Weak)
operation_succeeds();

// AFTER (Strong)
int before_count = get_resource_count();
operation_succeeds();
assert(get_resource_count() == before_count + 1);
assert(no_memory_leaks());
```

## Test Strength Checklist

A strong test should verify:
- [ ] **Specific outcomes**: Not just success/failure
- [ ] **Error messages**: Exact error text for failures  
- [ ] **Error locations**: Line numbers or positions
- [ ] **Resource management**: No leaks, proper cleanup
- [ ] **State changes**: All affected state is correct
- [ ] **Side effects**: Files created, messages sent, etc.
- [ ] **Edge cases**: Boundary conditions, NULL inputs
- [ ] **Failure paths**: What happens when things go wrong

## Using dlsym for Untestable Conditions

For conditions that can't be naturally triggered:
```c
// Use dlsym interception to test impossible scenarios
// See dlsym-test-interception-technique.md for details
```

## Best Practices

1. **One assertion per aspect**: Test each property separately
2. **Use descriptive assertions**: AR_ASSERT macros with messages
3. **Test the contract**: Verify documented behavior
4. **Test error paths**: At least one test per error condition
5. **Verify cleanup**: Ensure resources are freed on all paths
6. **Test boundaries**: NULL, empty, maximum values
7. **Chain verification**: Test complete workflows, not just units

## Related Patterns
- [Test Effectiveness Verification](test-effectiveness-verification.md)
- [AR Assert Descriptive Failures](ar-assert-descriptive-failures.md)
- [DLSym Test Interception Technique](dlsym-test-interception-technique.md)
- [Test Completeness Enumeration](test-completeness-enumeration.md)
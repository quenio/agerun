# Regression Test Removal Criteria

## Learning
Tests that verify the absence of permanently removed features should be deleted once the feature removal is architecturally complete and unlikely to be accidentally reintroduced.

## Importance
Keeping regression tests for permanently removed features:
- Adds unnecessary test execution time
- Creates confusion about system behavior
- Maintains code that references obsolete concepts
- Can mask real issues by testing for the wrong things
- Increases maintenance burden without providing value

## Example
```c
// OBSOLETE: Test verifying wake messages are NOT sent
static void test_no_wake_message_from_init_with_agent(void) {
    // This test verified the system doesn't send wake messages
    // But wake messages are permanently removed from the architecture
    ar_method_t *own_method = ar_method__create("init_test", 
        "memory.got_wake := if(message = \"__wake__\", 1, 0)", "1.0.0");
    ar_system__init_with_instance(mut_system, "init_test", "1.0.0");
    
    // Checking that wake was NOT received
    const ar_data_t *ref_got_wake = ar_data__get_map_data(ref_memory, "got_wake");
    AR_ASSERT(ref_got_wake == NULL, "Agent should NOT have received a wake message");
}
// DELETE: This entire test should be removed

// KEEP: Tests for current behavior
static void test_agent_initialization(void) {
    // Test what the system DOES do, not what it doesn't
    ar_system__init_with_instance(mut_system, "worker", "1.0.0");
    const ar_data_t *ref_memory = ar_agency__get_agent_memory_with_instance(mut_agency, 1);
    AR_ASSERT(ref_memory != NULL, "Agent should have memory");
    // Test actual initialization behavior
}
```

## Generalization
Remove regression tests when:
1. **Feature is architecturally removed**: Not just disabled but completely eliminated from codebase
2. **No code paths remain**: Grep confirms no references to the old feature exist
3. **Reintroduction is unlikely**: Would require deliberate re-implementation, not accidental inclusion
4. **Multiple cycles have passed**: Feature has been gone for several development cycles
5. **Documentation is updated**: All references to the feature are removed from docs

Keep regression tests when:
1. **Feature could return accidentally**: Simple typo or merge could reintroduce it
2. **External dependencies**: Third-party code might still send deprecated messages
3. **Backward compatibility**: System must handle old data formats gracefully
4. **Security boundaries**: Tests that verify dangerous operations don't occur

## Implementation
Process for identifying and removing obsolete regression tests:
```bash
# 1. Search for tests with "no_", "not_", "should_not" patterns
grep -r "test.*no_\|test.*not_\|should_not" *_tests.c

# 2. Review each to determine if it's testing absence of removed feature
# Look for patterns like:
# - "should NOT receive"
# - "no longer sends"  
# - "does not create"

# 3. Verify feature is completely gone
grep -r "wake\|sleep" modules/*.c  # Should return nothing

# 4. Remove test function and its declaration
# Also remove from test runner/main function

# 5. Run tests to ensure nothing breaks
make clean build 2>&1
```

## Related Patterns
- [Architectural Simplification Through Feature Removal](architectural-simplification-through-feature-removal.md)
- [Test Expectation Reality Alignment](test-expectation-reality-alignment.md)
- [Compilation-Based TDD Approach](compilation-based-tdd-approach.md)
- [Non-Functional Code Detection Pattern](non-functional-code-detection-pattern.md)
- [Feature Remnant Cleanup Pattern](feature-remnant-cleanup-pattern.md)
- [Commented Code Accumulation Antipattern](commented-code-accumulation-antipattern.md)
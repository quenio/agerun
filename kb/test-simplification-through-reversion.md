# Test Simplification Through Reversion

## Learning

When tests develop memory leaks or become overly complex while trying to fix issues, reverting to the original simpler structure often reveals that the complexity was the problem, not the solution. The principle "make the test as it was in the origin (regardless of whether it was correct or not)" can eliminate mysterious issues.

## Importance

This pattern prevents:
- Over-engineering test solutions
- Introducing new bugs while fixing old ones
- Hidden complexity that masks real problems
- Unnecessary test infrastructure that creates leaks

## Example

```c
// message_router_tests.c - Complex version with 15 memory leaks
static void test_router_handles_multiple_messages(void) {
    // Over-engineered with handler agent and payload
    int64_t handler = ar_agency__create_agent(mut_agency, 
                                                           "handler", "1.0.0", NULL);
    
    ar_data_t *own_payload = ar_data__create_map();
    ar_data__set_map_string(own_payload, "action", "test");
    ar_data__set_map_string(own_payload, "target", "handler");
    
    // Complex message structure
    ar_data_t *own_message = ar_data__create_map();
    ar_data__set_map_data(own_message, "payload", own_payload);
    // ... more complexity ...
}

// REVERTED to original simple version - 0 memory leaks
static void test_router_handles_multiple_messages(void) {
    // Simple direct approach
    ar_data_t *own_msg1 = ar_data__create_map();
    ar_data__set_map_string(own_msg1, "type", "echo");
    ar_data__set_map_string(own_msg1, "text", "Hello");
    
    ar_agent__send(router_agent, own_msg1);
    ar_method_fixture__process_next_message(own_fixture);
    
    // Direct verification without complex infrastructure
    ar_data_t *ref_last = ar_method_fixture__get_last_message(own_fixture);
    assert(strcmp(ar_data__get_map_string(ref_last, "echo"), "Hello") == 0);
}
```

## Generalization

When tests become problematic:
1. Check git history for the original simpler version
2. Revert to original structure first
3. Run tests to see if issues disappear
4. Only add complexity if original truly doesn't work
5. Each addition should be justified and tested

## Implementation

```bash
# Find original test implementation
git log -p methods/test_file.c | grep -A 50 "test_function_name"

# Compare current vs original complexity
git diff HEAD~10 methods/test_file.c | grep -E "create_|set_|destroy" | wc -l

# Revert specific test function
git checkout HEAD~10 -- methods/test_file.c
# Then manually extract just the test function

# Verify simplification fixes issues
make test_name 2>&1
grep "memory leaks" bin/run-tests/memory_report_test_name.log
```

## Related Patterns
- [Test Memory Leak Ownership Analysis](test-memory-leak-ownership-analysis.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
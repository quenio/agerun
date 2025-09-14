# Test Fixture API Adaptation

## Learning
When changing a public API that tests depend on, fixtures must be carefully redesigned to maintain test validity while adapting to the new interface. The fixture should provide the minimal abstraction needed to preserve test semantics.

## Importance
- Ensures tests continue to verify the same behaviors after API changes
- Prevents false positives from tests that pass but don't verify correctly
- Maintains test isolation and proper cleanup
- Enables gradual migration of test suites

## Example
```c
// Original fixture returned boolean
bool ar_interpreter_fixture__execute_instruction(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_instruction
);

// After API change, fixture returns agent ID for verification
int64_t ar_interpreter_fixture__execute_instruction(
    ar_interpreter_fixture_t *mut_fixture,
    const char *ref_instruction
) {
    // Create temporary method with the instruction
    char method_name[64];
    snprintf(method_name, sizeof(method_name), "temp_%ld", (long)time(NULL));
    
    ar_method_t *own_method = ar_method__create(
        method_name,
        ref_instruction,
        "1.0.0"
    );
    
    // Register and create agent
    ar_methodology__register_method(own_method);
    int64_t temp_agent_id = ar_agency__create_agent_with_instance(method_name, "1.0.0");
    
    // Execute and return agent ID for verification
    bool result = ar_interpreter__execute_method(
        mut_fixture->own_interpreter, 
        temp_agent_id, 
        NULL
    );
    
    return result ? temp_agent_id : 0;
}

// Add cleanup function for proper resource management
void ar_interpreter_fixture__destroy_temp_agent(
    ar_interpreter_fixture_t *mut_fixture,
    int64_t temp_agent_id
) {
    const char *method_name = NULL;
    const char *method_version = NULL;
    ar_agency__get_agent_method_with_instance(temp_agent_id, &method_name, &method_version);
    
    ar_agency__destroy_agent_with_instance(temp_agent_id);
    if (method_name && method_version) {
        ar_methodology__unregister_method(method_name, method_version);
    }
}
```

## Generalization
1. **Preserve test semantics**: Tests should verify the same behaviors
2. **Return verification handles**: Return IDs or handles instead of just success/failure
3. **Provide cleanup functions**: Match creation functions with destruction functions
4. **Minimize abstraction**: Only abstract what's necessary for the tests
5. **Document the adaptation**: Explain why the fixture design changed

## Implementation
```c
// Test adaptation pattern
int64_t handle = fixture_execute_operation(fixture, input);
assert(handle > 0);  // Verify success

// Use handle for verification
ar_data_t *result = fixture_get_result(fixture, handle);
assert(ar_data__get_integer(result) == expected_value);

// Clean up using handle
fixture_cleanup_operation(fixture, handle);
```

## Agent Context Creation Fix
When creating agents in fixtures, always provide a proper context instead of NULL:
```c
// WRONG: NULL context can cause expression evaluation failures
int64_t agent_id = ar_agency__create_agent_with_instance(method_name, version, NULL);

// CORRECT: Create empty map for context
ar_data_t *own_context = ar_data__create_map();
if (!own_context) {
    return 0;
}
int64_t agent_id = ar_agency__create_agent_with_instance(method_name, version, own_context);
// Agency takes ownership of context
```

This fix was critical for expression evaluation to work properly with context access.

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)
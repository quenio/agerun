# Frame Creation Prerequisites

## Learning
The ar_frame__create function requires ALL three parameters (memory, context, message) to be non-NULL. Passing NULL for any parameter results in frame creation failure with the error "Failed to create execution frame". This requirement isn't immediately obvious from the error message.

## Importance
- Prevents cryptic runtime errors during method execution
- Ensures proper initialization before agent execution
- Highlights importance of verifying function prerequisites
- Reveals hidden dependencies in execution flow
- Critical for debugging frame creation failures

## Example
```c
// WRONG: Passing NULL context causes frame creation failure
int64_t agent_id = ar_agency__create_agent(mut_agency, 
                                                        "test_method", 
                                                        "1.0.0", 
                                                        NULL);  // NULL context!

// Later when executing the method:
// ERROR: Failed to create execution frame
// (Because ar_frame__create received NULL context)

// RIGHT: Ensure all parameters are non-NULL
ar_data_t *own_context = ar_data__create_map();
ar_data_t *own_memory = ar_data__create_map();
ar_data_t *own_message = ar_data__create_string("test");

// Frame creation will succeed with all non-NULL parameters
ar_frame_t *own_frame = ar_frame__create(own_memory, own_context, own_message);
if (!own_frame) {
    // This would only happen if malloc fails, not due to NULL params
}

// For agents, ensure system provides context:
ar_system_t *own_system = ar_system__create();
// System internally creates shared context
int64_t agent_id = ar_system__init(own_system, "bootstrap", "1.0.0");
// Agent receives system's shared context, frame creation will work
```

## Generalization
When debugging "Failed to create X" errors:
1. Check ALL required parameters are non-NULL
2. Verify parameters are properly initialized
3. Trace parameter origin back to creation point
4. Look for NULL being passed through call chains
5. Check if parent components are providing required resources

## Implementation
```c
// Pattern for validating prerequisites before creation:
ar_frame_t* ar_frame__create(ar_data_t *mut_memory, 
                             const ar_data_t *ref_context,
                             const ar_data_t *ref_message) {
    // Validate ALL prerequisites
    if (!mut_memory || !ref_context || !ref_message) {
        ar_io__error("Frame creation requires non-NULL memory, context, and message");
        return NULL;
    }
    
    ar_frame_t *own_frame = AR__HEAP__MALLOC(sizeof(ar_frame_t));
    if (!own_frame) {
        return NULL;
    }
    
    // Initialize with validated parameters
    own_frame->mut_memory = mut_memory;
    own_frame->ref_context = ref_context;
    own_frame->ref_message = ref_message;
    
    return own_frame;
}
```

## Related Patterns
- [Shared Context Architecture Pattern](shared-context-architecture-pattern.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [API Ownership on Failure](api-ownership-on-failure.md)
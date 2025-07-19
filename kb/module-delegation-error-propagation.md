# Module Delegation with Error Propagation

## Learning
When one module delegates execution to another (like interpreter delegating to method evaluator), proper error propagation requires passing a log instance through the delegation chain and using return values to indicate success/failure.

## Importance
This pattern enables clean separation of concerns while maintaining comprehensive error reporting. It allows high-level modules to delegate complex operations while still providing detailed error information to users.

## Example
```c
// From ar_interpreter.c - Delegating to method evaluator
bool ar_interpreter__execute_method(ar_interpreter_t *mut_interpreter,
                                   int64_t agent_id, 
                                   const ar_data_t *ref_message) {
    // Retrieve method from agent
    ar_method_t *ref_method = ar_agency__get_agent_method(agent_id);
    if (!ref_method) {
        ar_log__error(mut_interpreter->ref_log, 0, 
                     "Agent %ld has no method", agent_id);
        return false;
    }
    
    // Retrieve agent's memory and context
    ar_data_t *mut_memory = ar_agency__get_agent_mutable_memory(agent_id);
    const ar_data_t *ref_context = ar_agency__get_agent_context(agent_id);
    
    // Create frame for execution
    ar_frame_t *own_frame = ar_frame__create(mut_memory, ref_context, ref_message);
    if (!own_frame) {
        ar_log__error(mut_interpreter->ref_log, 0, "Failed to create frame");
        return false;
    }
    
    // Delegate to method evaluator with log for error reporting
    bool success = ar_method_evaluator__evaluate(
        mut_interpreter->mut_method_evaluator,
        ref_method,
        own_frame
    );
    
    ar_frame__destroy(own_frame);
    return success;  // Propagate success/failure
}

// From ar_interpreter__create - Passing log through layers
ar_interpreter_t* ar_interpreter__create(ar_log_t *ref_log) {
    ar_interpreter_t *own_interpreter = AR__HEAP__MALLOC(sizeof(ar_interpreter_t));
    if (!own_interpreter) {
        return NULL;
    }
    
    own_interpreter->ref_log = ref_log;  // Store log reference
    
    // Create method evaluator with same log instance
    own_interpreter->mut_method_evaluator = ar_method_evaluator__create(ref_log);
    if (!own_interpreter->mut_method_evaluator) {
        AR__HEAP__FREE(own_interpreter);
        return NULL;
    }
    
    return own_interpreter;
}
```

## Generalization
The delegation pattern follows these principles:
1. **Log parameter propagation**: Pass log instance through all layers
2. **Return value semantics**: Use bool/status codes for success/failure
3. **Error details in log**: Detailed messages go to log, not return values
4. **Resource retrieval**: Higher layer retrieves resources for lower layer
5. **Clean handoff**: Create necessary context (like frame) for delegation

## Implementation
```c
// Pattern for creating delegating module
typedef struct {
    ar_log_t *ref_log;                              // Borrowed log reference
    ar_method_evaluator_t *mut_method_evaluator;    // Owned sub-module
} ar_interpreter_t;  // Real example from interpreter

ar_interpreter_t* ar_interpreter__create(ar_log_t *ref_log) {
    ar_interpreter_t *interpreter = AR__HEAP__MALLOC(sizeof(ar_interpreter_t));
    interpreter->ref_log = ref_log;
    
    // Create sub-module with same log
    interpreter->mut_method_evaluator = ar_method_evaluator__create(ref_log);
    return interpreter;
}

// Pattern for delegation
bool ar_interpreter__execute_method(ar_interpreter_t *interpreter, 
                                   int64_t agent_id,
                                   const ar_data_t *ref_message) {
    // Validate inputs
    if (!interpreter || agent_id <= 0) {
        ar_log__error(interpreter->ref_log, 0, "Invalid parameters");
        return false;
    }
    
    // Prepare context for sub-module
    ar_frame_t *frame = ar_frame__create(memory, context, ref_message);
    
    // Delegate to sub-module
    bool success = ar_method_evaluator__evaluate(
        interpreter->mut_method_evaluator, method, frame);
    
    // Clean up
    ar_frame__destroy(frame);
    
    return success;  // Propagate result
}
```

## Related Patterns
- [Facade Pattern Coordination](facade-pattern-coordination.md)
- [Error Propagation Pattern](error-propagation-pattern.md)
- [Architectural Patterns Hierarchy](architectural-patterns-hierarchy.md)
- [Separation of Concerns Principle](separation-of-concerns-principle.md)
- [Instruction Behavior Simplification](instruction-behavior-simplification.md)
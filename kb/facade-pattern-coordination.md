# Facade Pattern Coordination

## Learning
Facades should ONLY coordinate and delegate, never implement business logic directly

## Importance
Maintains clean separation between coordination and implementation, enables independent testing of components

## Example
```c
// Good: Facade coordinates
bool ar_instruction_evaluator__evaluate(evaluator, frame, ast) {
    ar_instruction_type_t type = ar_instruction_ast__get_type(ast);
    switch (type) {
        case AR_INSTRUCTION_TYPE__ASSIGNMENT:
            return ar_assignment_instruction_evaluator__evaluate(
                evaluator->assignment_eval, frame, ast);
        case AR_INSTRUCTION_TYPE__SEND:
            return ar_send_instruction_evaluator__evaluate(
                evaluator->send_eval, frame, ast);
        // ... delegate to specialized evaluators
    }
}
```

## Generalization
Use facades to coordinate multiple specialized components rather than implementing functionality

## Implementation
- **Coordinate, don't implement**: Delegate all business logic
- **Update creation when interfaces change**: Keep facade current
- **Run facade tests after sub-component changes**: Verify integration
- **Frame-based evaluators**: Create upfront, not lazily
- **Minimal interface**: Expose only coordination methods

## Interpreter-Method Evaluator Example
The interpreter module demonstrates perfect facade coordination by delegating to method evaluator:
```c
// From ar_interpreter.c - Pure delegation
bool ar_interpreter__execute_method(ar_interpreter_t *mut_interpreter,
                                   int64_t agent_id, 
                                   const ar_data_t *ref_message) {
    // Retrieve resources
    ar_method_t *ref_method = ar_agency__get_agent_method(agent_id);
    ar_data_t *mut_memory = ar_agency__get_agent_mutable_memory(agent_id);
    const ar_data_t *ref_context = ar_agency__get_agent_context(agent_id);
    
    // Create frame for delegation
    ar_frame_t *own_frame = ar_frame__create(mut_memory, ref_context, ref_message);
    
    // Delegate execution - no business logic here
    bool success = ar_method_evaluator__evaluate(
        mut_interpreter->mut_method_evaluator,
        ref_method,
        own_frame
    );
    
    ar_frame__destroy(own_frame);
    return success;
}
```

## Related Patterns
- Delegation over implementation
- Component coordination
- Interface segregation
- Dependency injection
- Architectural boundaries
- [Module Delegation Error Propagation](module-delegation-error-propagation.md)
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

## Related Patterns
- Delegation over implementation
- Component coordination
- Interface segregation
- Dependency injection
- Architectural boundaries
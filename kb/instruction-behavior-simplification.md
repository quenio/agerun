# Instruction Behavior Simplification

## Learning
Complex instruction behaviors can be simplified by removing side effects and focusing on a single responsibility. This makes the code easier to test, understand, and maintain.

## Importance
Instructions that do too much (like destroying agents AND unregistering methods) create tight coupling, complicate testing, and make the system harder to reason about. Simplifying to single responsibilities improves code quality.

## Example
```c
// BEFORE: Deprecate instruction did too much
// From old ar_deprecate_instruction_evaluator.c
bool evaluate_deprecate(evaluator, method_name, version) {
    // Complex behavior:
    // 1. Get method and count agents using it
    ar_method_t *method = ar_methodology__get_method(method_name, version);
    int count = ar_agency__count_agents_using_method(method);
    
    // 2. Send sleep messages to all agents
    // (iterate through all agents, check if using this method)
    
    // 3. Process all sleep messages
    while (ar_system__process_next_message_with_instance(own_system));
    
    // 4. Destroy all agents using this method
    // (iterate and destroy each one)
    
    // 5. Finally unregister the method
    return ar_methodology__unregister_method(method_name, version);
}

// AFTER: Simplified to single responsibility
// From new ar_deprecate_instruction_evaluator.c
bool evaluate_deprecate(evaluator, method_name, version) {
    // Simple behavior: just unregister the method
    ar_method_t *ref_method = ar_methodology__get_method(method_name, version);
    if (ref_method) {
        return ar_methodology__unregister_method(method_name, version);
    }
    return false;
}

// Also simplified ar_methodology__unregister_method
// BEFORE: Checked if agents were using the method
bool ar_methodology__unregister_method(name, version) {
    if (ar_agency__count_agents_using_method(name, version) > 0) {
        return false;  // Refused to unregister
    }
    // ... unregister logic
}

// AFTER: Allows unregistering regardless of usage
bool ar_methodology__unregister_method(name, version) {
    // Simply unregister - no side effects or checks
    // ... unregister logic
    return true;
}
```

## Generalization
Signs that an instruction needs simplification:
1. **Multiple responsibilities**: Does more than one distinct operation
2. **Side effects**: Triggers cascading changes in other modules
3. **Complex testing**: Requires elaborate setup to test all paths
4. **Unclear semantics**: Name doesn't clearly convey all behaviors
5. **Cross-module coupling**: Reaches into multiple modules

Simplification strategies:
- **Extract side effects**: Move them to separate instructions/functions
- **Single responsibility**: Each instruction does one thing well
- **Clear naming**: Name should describe the single action
- **Minimal coupling**: Interact with fewest modules possible

## Implementation
```c
// Pattern: Identify complex instruction
void analyze_instruction() {
    // Count responsibilities
    // 1. Primary action (e.g., unregister)
    // 2. Side effect 1 (e.g., send messages)  
    // 3. Side effect 2 (e.g., destroy entities)
    // If count > 1, consider simplification
}

// Pattern: Refactor to single responsibility
// Before
bool complex_instruction() {
    do_action_a();
    do_action_b();
    do_action_c();
    return success;
}

// After - Split into separate instructions
bool simple_instruction_a() {
    return do_action_a();
}

bool simple_instruction_b() {
    return do_action_b();
}

// Let caller orchestrate if needed
if (simple_instruction_a()) {
    simple_instruction_b();
}
```


**Note**: Examples assume `own_system`, `mut_agency`, and other instance variables are available. In practice, these would be created via fixtures or passed as parameters.
## Related Patterns
- [Single Responsibility Principle](single-responsibility-principle.md)
- [Refactoring Key Patterns](refactoring-key-patterns.md)
- [Module Delegation Error Propagation](module-delegation-error-propagation.md)
- [Design for Change Principle](design-for-change-principle.md)
- [Separation of Concerns Principle](separation-of-concerns-principle.md)
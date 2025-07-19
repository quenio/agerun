# Architectural Patterns Hierarchy

## Learning
Preferred architectural patterns for resolving circular dependencies and improving code organization, ranked by preference.

## Importance
Using the right pattern for each situation prevents overengineering while maintaining clean architecture and avoiding circular dependencies.

## Example
```c
// 1. Interface Segregation - Split large modules
// Before: Single 850-line agency module
// After: 
// - ar_agent_registry.c (lifecycle management)
// - ar_agent_store.c (persistence)  
// - ar_agent_update.c (state changes)
// - ar_agency.c (core coordination)

// 2. Registry Pattern - Central ownership
typedef struct ar_agent_registry_s {
    ar_map_t *own_agents;  // Registry owns all agents
} ar_agent_registry_t;

ar_agent_t* ar_agent_registry__get(ar_agent_registry_t *ref_registry, uint64_t id) {
    return ar_map__get(ref_registry->own_agents, id);  // Returns reference
}

// 3. Facade Pattern - Coordinate only, no business logic
ar_data_t* ar_instruction_facade__evaluate(ar_instruction_ast_t *ast, ar_data_t *memory) {
    // ONLY coordinate between specialized evaluators
    switch (ast->type) {
        case ASSIGNMENT:
            return ar_assignment_evaluator__evaluate(ast, memory);
        case SEND:
            return ar_send_evaluator__evaluate(ast, memory);
        // NO business logic here - only delegation
    }
}

// 4. Parser/Executor Split
// ar_method_parser.c - Parses method syntax
// ar_method_executor.c - Executes parsed methods
// Clear separation of concerns

// 5. Callbacks/DI - Last resort due to complexity
typedef ar_data_t* (*evaluator_fn)(ar_instruction_ast_t*, ar_data_t*);
```

## Generalization
1. **Interface Segregation**: Split large modules (agency → registry/store/update)
2. **Registry Pattern**: Central ownership of lifecycle (registry owns all agents)
3. **Facade Pattern**: ONLY coordinate, never implement business logic; update creation when interfaces change; run facade tests after sub-component changes; frame-based evaluators→create upfront, not lazily
4. **Parser/Executor Split**: Separate concerns for clarity
5. **Callbacks/DI**: Last resort - adds complexity

## Implementation
Choose patterns based on the problem:
- **Too many responsibilities**: Use interface segregation
- **Ownership confusion**: Use registry pattern
- **Complex coordination**: Use facade pattern
- **Mixed parsing/execution**: Split into separate modules
- **Dynamic behavior**: Consider callbacks only after other options exhausted

Always prefer simpler patterns first. Move to complex patterns only when simpler ones don't solve the architectural issue.

## Related Patterns
- [Facade Pattern Coordination](facade-pattern-coordination.md)
- [No Circular Dependencies Principle](no-circular-dependencies-principle.md)
- [Single Responsibility Principle](single-responsibility-principle.md)
- [Module Delegation Error Propagation](module-delegation-error-propagation.md)
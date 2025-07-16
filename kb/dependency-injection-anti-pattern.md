# Dependency Injection Anti-pattern

## Learning
Dependency injection becomes an anti-pattern when there's a 1:1 relationship between components with no alternate implementations. In such cases, having the parent component create its dependencies internally simplifies the API and reduces complexity.

## Importance
- Simplifies component APIs by reducing parameters
- Eliminates unnecessary abstraction layers
- Makes code easier to understand and use
- Reduces boilerplate in tests and client code

## Example
```c
// Before: Unnecessary dependency injection
ar_instruction_evaluator_t* evaluator = ar_instruction_evaluator__create(
    ref_log,
    ref_expr_evaluator  // Only one implementation exists
);

// After: Internal creation
ar_instruction_evaluator_t* evaluator = ar_instruction_evaluator__create(ref_log);

// Implementation creates its dependency internally
struct ar_instruction_evaluator_s {
    ar_log_t *ref_log;
    ar_expression_evaluator_t *own_expr_evaluator;  // Created internally
};

ar_instruction_evaluator_t* ar_instruction_evaluator__create(ar_log_t *ref_log) {
    // ... allocate evaluator ...
    
    // Create the expression evaluator internally
    evaluator->own_expr_evaluator = ar_expression_evaluator__create(ref_log);
    if (evaluator->own_expr_evaluator == NULL) {
        AR__HEAP__FREE(evaluator);
        return NULL;
    }
    
    return evaluator;
}
```

## Generalization
Before using dependency injection, ask:
1. Are there multiple implementations of this dependency?
2. Will there likely be alternate implementations in the future?
3. Does the client need to customize the dependency?

If all answers are "no", create the dependency internally instead.

## Implementation
1. Change struct field from borrowed (ref_) to owned (own_)
2. Remove dependency parameter from create function
3. Create dependency internally in constructor
4. Destroy owned dependency in destructor
5. Update all tests to use simplified API

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Management Model](../MMM.md)
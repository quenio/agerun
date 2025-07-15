# Expression Ownership Rules

## Learning
Understanding ownership semantics for different types of expression evaluations to prevent memory leaks and use-after-free errors.

## Importance
Incorrect ownership assumptions are a primary source of memory leaks in AgeRun. Knowing when expressions return owned vs borrowed values is critical.

## Example
```c
// Memory access returns REFERENCE - do NOT destroy
ar_data_t *ref_value = ar_expression_evaluator__evaluate("memory.x", memory);
// ref_value is borrowed from memory - no ar_data__destroy() needed

// Arithmetic creates NEW object - MUST destroy
ar_data_t *own_sum = ar_expression_evaluator__evaluate("2 + 3", memory);
// ... use sum ...
ar_data__destroy(own_sum);  // Required cleanup

// String operations create NEW object - MUST destroy  
ar_data_t *own_concat = ar_expression_evaluator__evaluate("\"Hello\" + \" World\"", memory);
// ... use concat ...
ar_data__destroy(own_concat);  // Required cleanup

// Map iteration pattern
ar_data_t *own_keys = ar_data__get_map_keys(ref_map);  // Creates new list
ar_data_t *ref_current = ar_data__list_first(own_keys);
while (ref_current) {
    // Use ref_current (do NOT destroy individual items here)
    ref_current = ar_data__list_next(ref_current);  // EXAMPLE: Hypothetical iteration
}
// Must manually destroy string elements before destroying list
size_t count = ar_data__list_count(own_keys);
for (size_t i = 0; i < count; i++) {
    // Note: No direct index access - would need to iterate and track position
    // This is a limitation of current API - use ar_data__list_first/next pattern instead
}
ar_data__destroy(own_keys);  // Destroys the list structure
```

## Generalization
- **Memory access** (`memory.x`, `memory.y.z`): Returns reference from memory/context
- **Arithmetic** (`2 + 3`, `memory.x + 5`): Creates new object that must be destroyed
- **String operations** (`"a" + "b"`, string functions): Create new objects
- **Map keys**: `ar_data__get_map_keys()` creates new list with owned string elements
- **List items**: `ar_data__list_first()` returns references (don't destroy)
- **Persistence**: Write key/type on one line, value on next for proper parsing

## Implementation
1. Check expression type to determine ownership
2. Use proper variable prefix (own_ vs ref_) immediately
3. Add cleanup for all owned values before function exit
4. Never destroy borrowed references from memory/context
5. Use `ar_data__hold_ownership()` to debug ownership status

**Self-ownership check**: Never let `values_result == mut_memory` in evaluators
**Context lifetime**: NEVER destroy context or its elements in evaluators

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
# Refactoring Ownership Transitions

## Learning
When refactoring code that changes ownership semantics (from borrowed to owned references), immediately update variable naming to reflect the new ownership. This maintains clarity and prevents memory management errors.

## Importance
- Self-documenting code through consistent naming
- Prevents memory leaks by clarifying ownership
- Makes review easier by highlighting ownership changes
- Ensures proper cleanup in destructors

## Example
```c
// Before: Borrowed reference
struct ar_instruction_evaluator_s {
    ar_log_t *ref_log;
    ar_expression_evaluator_t *ref_expr_evaluator;  // Borrowed
};

// After: Owned reference with immediate naming update
struct ar_instruction_evaluator_s {
    ar_log_t *ref_log;
    ar_expression_evaluator_t *own_expr_evaluator;  // Owned - name changed
};

// Destructor must now clean up owned reference
void ar_instruction_evaluator__destroy(ar_instruction_evaluator_t *own_evaluator) {
    if (own_evaluator == NULL) {
        return;
    }
    
    // Destroy the owned expression evaluator
    if (own_evaluator->own_expr_evaluator != NULL) {
        ar_expression_evaluator__destroy(own_evaluator->own_expr_evaluator);
    }
    
    AR__HEAP__FREE(own_evaluator);
}
```

## Generalization
When changing ownership during refactoring:
1. Update variable/field names immediately (ref_ → own_ or mut_)
2. Add ownership transfer comments at return statements
3. Update destructors to clean up newly owned resources
4. Set pointers to NULL after ownership transfer
5. Verify with memory leak detection tools

## Implementation
```bash
# Find all fields that need renaming
grep -n "ref_.*evaluator" modules/*.c

# After refactoring, verify no leaks
make sanitize-tests
grep "Actual memory leaks:" bin/memory_report_*.log | grep -v "0 (0 bytes)"
```

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
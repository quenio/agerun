# Test Function Naming Accuracy

## Learning
Test function names should reflect actual behavior, not original intent

## Importance
Misleading test names make codebase harder to understand

## Example
"wrong_type" tests actually tested positive cases after API unification

## Generalization
Review test names when behavior changes during refactoring

## Implementation
```c
// Before: test_evaluate_literal_int_wrong_type
// After:  test_evaluate_handles_string_literal

// Names should describe what the test actually verifies
static void test_evaluate_handles_string_literal(void) {
    // Test that evaluate function correctly handles string literals
}
```

## Related Patterns
- Test names are documentation of expected behavior
- Refactoring may change test purpose without changing test name
- Clear test names improve code maintainability
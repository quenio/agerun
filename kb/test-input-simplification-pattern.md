# Test Input Simplification Pattern

## Learning
Tests should use the simplest possible inputs that demonstrate the behavior being tested. Using literal values directly is preferable to complex memory access expressions when testing specific functionality.

## Importance
Simplified test inputs provide:
- Clearer test intent and readability
- Easier debugging when tests fail
- Reduced coupling between test setup and test execution
- Focus on the specific behavior being tested
- Fewer potential points of failure

## Example
```c
// BAD: Overly complex test setup with memory indirection
ar_data__set_map_data(memory, "method_name", ar_data__create_integer(0));
ar_expression_ast_t *method_ast = ar_expression_ast__create_memory_access(
    "memory", "method_name", 1  // Complex memory access
);

// GOOD: Direct literal values
ar_expression_ast_t *method_ast = ar_expression_ast__create_literal_int(0);
// Clear, direct, and focuses on testing the no-op behavior
```

Another example:
```c
// BAD: Passing entire memory structure as context
ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access(
    "memory", NULL, 0  // Passing whole memory map
);

// GOOD: Using the standard context variable
ar_expression_ast_t *context_ast = ar_expression_ast__create_memory_access(
    "context", NULL, 0  // Using proper context variable
);
```

## Generalization
When writing tests:
1. Use literal values when testing specific behaviors
2. Avoid unnecessary indirection through memory structures
3. Set up only the minimum required state
4. Use standard variables (context, message) appropriately
5. Reserve complex setups for integration tests that specifically need them

## Implementation
```bash
# Review existing tests for simplification opportunities
grep -r "create_memory_access.*memory\." modules/*_tests.c

# Replace with literals where appropriate
# Use ar_expression_ast__create_literal_int()
# Use ar_expression_ast__create_literal_string()
```

## Related Patterns
- [Test String Selection Strategy](test-string-selection-strategy.md)
- [BDD Test Structure](bdd-test-structure.md)
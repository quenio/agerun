# Compilation-Based TDD Approach

## Context

Traditional TDD uses failing tests as the RED phase. However, when removing infrastructure code (like global constants or function parameters), behavior tests may not be possible. In these cases, we can use compilation failure as our RED phase test.

## The Pattern

### RED Phase: Create Compilation Failure
Instead of writing a failing test, remove the code that will cause compilation to fail:
```c
// Remove the constant definition
// static const char *g_sleep_message = "__sleep__";  // Commented out

// This causes compilation errors wherever g_sleep_message is used
```

### GREEN Phase: Fix Compilation
Remove all references to make the code compile:
```c
// Remove usage of g_sleep_message
// ar_data_t *own_sleep_msg = ar_data__create_string(g_sleep_message);
// Entire block removed
```

### REFACTOR Phase: Clean Up
Remove unused parameters and simplify:
```c
// Before:
bool ar_agent__update_method(ar_agent_t *mut_agent, const ar_method_t *ref_new_method, bool send_sleep_wake);

// After:
bool ar_agent__update_method(ar_agent_t *mut_agent, const ar_method_t *ref_new_method);
```

## When to Use This Approach

1. **Removing global constants**: When constants are used across multiple files
2. **Removing function parameters**: When parameters affect multiple callers
3. **Removing struct fields**: When fields are accessed in multiple places
4. **Infrastructure changes**: When the change affects compilation rather than runtime behavior

## Benefits

1. **Compiler as test runner**: The compiler verifies all usage sites are updated
2. **Comprehensive coverage**: Can't miss any usage sites (compilation would fail)
3. **Fast feedback**: Compilation is faster than running full test suite
4. **Safe refactoring**: No hidden runtime failures

## Example from AgeRun

Removing sleep messages using compilation-based TDD:

### Iteration 3.1: Remove sleep from destruction
- RED: Comment out sleep message sending → compilation fails
- GREEN: Remove the code block → compilation succeeds
- REFACTOR: Clean up empty if statement

### Iteration 3.2: Remove send_sleep_wake parameter
- RED: Remove parameter from function signature → compilation fails
- GREEN: Update all callers → compilation succeeds
- REFACTOR: Simplify function body

### Iteration 3.3: Remove global constant
- RED: Remove g_sleep_message constant → compilation fails
- GREEN: Remove all usages → compilation succeeds
- REFACTOR: Clean up related code

## Key Principles

1. **One change at a time**: Each iteration should focus on a single compilation unit
2. **Verify tests still pass**: After GREEN phase, ensure behavior is preserved
3. **Document non-functional discoveries**: If code was non-functional, document it
4. **Complete the cycle**: Don't skip REFACTOR phase - clean up thoroughly

## Comparison with Traditional TDD

| Aspect | Traditional TDD | Compilation-Based TDD |
|--------|----------------|----------------------|
| RED trigger | Failing test | Compilation failure |
| GREEN goal | Test passes | Code compiles |
| REFACTOR scope | Improve design | Remove unused code |
| Best for | Adding features | Removing infrastructure |

## Important Notes

- Still run full test suite after each cycle to ensure no behavior changes
- Memory leak detection remains critical
- Document why compilation-based approach was chosen
- Consider if behavior tests could be added for future protection

## Related Patterns

- [Non-Functional Code Detection Pattern](non-functional-code-detection-pattern.md) - Identifying code that can be safely removed
- [Systematic Parameter Removal Refactoring](systematic-parameter-removal-refactoring.md) - Removing parameters using compilation-based approach
- [Red Green Refactor Cycle](red-green-refactor-cycle.md) - Traditional TDD cycle comparison
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md) - Comprehensive TDD methodology
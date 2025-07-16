# TDD for API Simplification

## Learning
When simplifying APIs through refactoring, maintain strict TDD discipline even for seemingly straightforward changes. Each modification should follow the Red-Green-Refactor cycle to ensure no behavior is accidentally changed.

## Importance
- Prevents regression bugs during refactoring
- Ensures all existing functionality is maintained
- Provides confidence that the API change is safe
- Documents the change process through failing tests

## Example
```c
// TDD Cycle for removing dependency injection

// RED Phase: Modify test to use new API (will fail to compile)
static void test_instruction_evaluator__create_destroy(void) {
    ar_log_t *log = ar_log__create();
    
    // This will fail - API doesn't accept single parameter yet
    ar_instruction_evaluator_t *evaluator = ar_instruction_evaluator__create(log);
    
    assert(evaluator != NULL);
    ar_instruction_evaluator__destroy(evaluator);
    ar_log__destroy(log);
}

// GREEN Phase: Update implementation to make test pass
ar_instruction_evaluator_t* ar_instruction_evaluator__create(ar_log_t *ref_log) {
    // Implementation that creates expression evaluator internally
    // ...
}

// REFACTOR Phase: Clean up code while keeping tests green
// - Remove obsolete tests
// - Update documentation
// - Ensure consistent naming
```

## Generalization
For API simplification refactoring:
1. Write/modify test with desired new API (RED)
2. Update implementation minimally to pass (GREEN)
3. Clean up and improve while keeping tests passing (REFACTOR)
4. Repeat for each affected component

## Implementation
1. Identify all tests using the old API
2. Create TDD plan with one cycle per test/component
3. Follow Red-Green-Refactor strictly for each cycle
4. Only commit after ALL cycles complete
5. Run full test suite including memory checks

## Related Patterns
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
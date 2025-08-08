# TDD Cycle Detailed Explanation

## Learning

The Red-Green-Refactor cycle is not just a suggestion but a mandatory three-phase process that must be completed for each behavior before moving to the next.

## Importance

Following all three phases ensures code quality, prevents technical debt, and maintains clean architecture. Skipping the refactor phase leads to accumulated complexity and harder maintenance.

## Example

```c
// CYCLE 1: Create functionality
// Red Phase - Write failing test
void test_ar_data__create_string() {
    ar_data_t* own_data = ar_data__create_string("home.config");
    assert(own_data != NULL);
    assert(strcmp(ar_data__get_string(own_data), "home.config") == 0);
    ar_data__destroy(own_data);
}

// Green Phase - Minimal implementation
ar_data_t* ar_data__create_string(const char* ref_string) {
    ar_data_t* own_data = AR__HEAP__MALLOC(sizeof(ar_data_t));
    own_data->type = AR_DATA_TYPE_STRING;
    own_data->value.string = AR__HEAP__STRDUP(ref_string);
    return own_data;  // Ownership transferred to caller
}

// Refactor Phase - Extract helper, improve naming
static void _validate_string_input(const char* ref_input) {
    // Extracted validation logic
}

// CYCLE 2: Add evaluation - ONLY after Cycle 1 complete
// Continue with next behavior...
```

## Generalization

For each new behavior:
1. **Red**: Test must ACTUALLY FAIL (not just compile error)
2. **Green**: Write MINIMUM code - resist adding features
3. **Refactor**: MANDATORY phase - state "No improvements identified" if none needed

Key rules:
- NO commits during cycles
- Complete ALL cycles before documentation
- One commit per feature, not per cycle
- Process wake messages in tests to prevent leaks

## Implementation

```bash
# Run test to see failure
make test_module 2>&1  # Should show test failure

# After implementation
make test_module 2>&1  # Should pass
grep "Actual memory leaks: 0" bin/run-tests/memory_report_test_module.log

# After refactoring  
make test_module 2>&1  # Should still pass

# Only after ALL cycles
git add -A
git commit -m "feat: implement feature using TDD

- Completed N TDD cycles for behaviors X, Y, Z
- All tests passing with zero memory leaks
- Updated documentation"
```

## Related Patterns
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [TDD API Simplification](tdd-api-simplification.md)
- [TDD Feature Completion Before Commit](tdd-feature-completion-before-commit.md)
- [Requirement Precision in TDD](requirement-precision-in-tdd.md)
# TDD Cycle Detailed Explanation

## Learning

The Red-Green-Refactor cycle is not just a suggestion but a mandatory three-phase process that must be completed for each behavior before moving to the next.

### Terminology: Cycle vs Iteration

In TDD terminology, a **cycle** is the overall unit of work (e.g., "TDD Cycle 6.5: Message Queue Infrastructure"), while **iteration** refers to individual RED-GREEN-REFACTOR sequences within that cycle. Each iteration implements one specific assertion.

**Example hierarchy:**
```
TDD Cycle 6.5: Message Queue Infrastructure (the cycle)
├── Iteration 1: send() returns true (one RED-GREEN-REFACTOR)
├── Iteration 2: has_messages() returns false initially (one RED-GREEN-REFACTOR)
├── Iteration 3: has_messages() returns true after send() (one RED-GREEN-REFACTOR)
└── ... (more iterations)
```

Use precise terminology: "Cycle 6.5 has 14 iterations" not "Cycle 6.5 has 14 cycles".

## Importance

Following all three phases ensures code quality, prevents technical debt, and maintains clean architecture. Skipping the refactor phase leads to accumulated complexity and harder maintenance.

## Example

```c
// ITERATION 1: Create functionality
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

// ITERATION 2: Add evaluation - ONLY after Iteration 1 complete
// Continue with next behavior...
```

## Generalization

For each new behavior:
1. **Red**: Test must ACTUALLY FAIL (not just compile error)
2. **Green**: Write MINIMUM code - resist adding features
3. **Refactor**: MANDATORY phase - state "No improvements identified" if none needed

Key rules:
- NO commits during iterations
- Complete ALL iterations before documentation
- One commit per cycle (feature), not per iteration
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

# Only after ALL iterations
git add -A
git commit -m "feat: implement feature using TDD

- Completed N TDD iterations for behaviors X, Y, Z
- All tests passing with zero memory leaks
- Updated documentation"
```

## Related Patterns
- [TDD Iteration Planning Pattern](tdd-iteration-planning-pattern.md) - One assertion per iteration
- [TDD GREEN Phase Minimalism](tdd-green-phase-minimalism.md) - Minimal implementations
- [TDD RED Phase Assertion Requirement](tdd-red-phase-assertion-requirement.md) - Proper RED phases
- [Iterative Plan Refinement Pattern](iterative-plan-refinement-pattern.md) - Refining TDD plans
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [TDD API Simplification](tdd-api-simplification.md)
- [TDD Feature Completion Before Commit](tdd-feature-completion-before-commit.md)
- [Requirement Precision in TDD](requirement-precision-in-tdd.md)
- [Documentation-Only Change Pattern](documentation-only-change-pattern.md)
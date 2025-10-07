# Red-Green-Refactor Cycle

## Learning
TDD requires completing ALL three phases (Red-Green-Refactor) for each behavior before moving to the next, with no commits during the cycle. When implementing a feature with multiple behaviors, complete ALL cycles for the entire feature before committing.

## Importance
Ensures code quality through systematic improvement and prevents accumulation of technical debt. Prevents incomplete features in commits and maintains clean git history.

## Example
```
For adding method parameter validation:

RED: Write test that expects validation error → Test FAILS
GREEN: Add minimal validation code → Test PASSES  
REFACTOR: Extract validation helper, improve naming → Tests still PASS

Only after ALL three phases: Commit the complete feature
```

## Generalization
Break complex features into small behavior cycles, completing each cycle fully before advancing

## Implementation
1. **Red Phase**: Write REAL failing test that verifies actual behavior
   - Test must actually execute and FAIL (not just compile error)
   - No placeholder tests that do nothing
   - Test failure must be for the RIGHT reason
2. **Green Phase**: Write minimum code to make test pass
3. **Refactor Phase**: Improve code while keeping tests green (MANDATORY)
4. **Repeat cycle** for next behavior
5. **Commit only** after ALL cycles complete

Critical: 
- RED phase must be a real test, not a placeholder
- Refactor phase is NOT optional - always look for improvements
- NEVER skip phases even if solution seems obvious - maintains discipline

### Example: Extracting Ownership Patterns
```c
// RED: Test for ownership utility function
void test_data__claim_or_copy() {
    ar_data_t *data = ar_data__create_integer(42);
    ar_data_t *result = ar_data__claim_or_copy(data, owner);  // FAILS - function doesn't exist
}

// GREEN: Implement minimal function
ar_data_t* ar_data__claim_or_copy(ar_data_t *ref_data, void *owner) {
    if (ar_data__take_ownership(ref_data, owner)) {
        ar_data__drop_ownership(ref_data, owner);
        return ref_data;
    }
    return ar_data__shallow_copy(ref_data);
}

// REFACTOR: Add null checks, handle edge cases
ar_data_t* ar_data__claim_or_copy(ar_data_t *ref_data, void *owner) {
    if (!ref_data || !owner) return NULL;  // Added in refactor
    // ... rest of implementation
}
```

## Related Patterns
- [BDD Test Structure](bdd-test-structure.md)
- [Test Assertion Strength Patterns](test-assertion-strength-patterns.md)
- [Standards Over Expediency Principle](standards-over-expediency-principle.md)
- [KB Consultation Before Planning Requirement](kb-consultation-before-planning-requirement.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)
- [Integration Test Binary Execution](integration-test-binary-execution.md)
- [Permission-Based Test Modification](permission-based-test-modification.md)
- Test-driven development methodology
- Incremental feature development
- Code quality maintenance
- Systematic improvement processes
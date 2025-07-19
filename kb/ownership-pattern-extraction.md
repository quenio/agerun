# Ownership Pattern Extraction

## Learning
Complex ownership patterns that appear across multiple files can be safely extracted into reusable functions using TDD methodology. The key is identifying the common pattern variations and creating functions that handle all cases.

## Importance
Ownership patterns are critical for memory safety in C. Duplicated patterns across files create maintenance burden and increase the risk of memory leaks. Extracting these patterns reduces code duplication and ensures consistent behavior.

## Example
Common ownership pattern found in evaluators:
```c
// Pattern 1: Take ownership or create copy
ar_data_t *own_data;
if (ar_data__take_ownership(result, evaluator)) {
    ar_data__drop_ownership(result, evaluator);
    own_data = result;
} else {
    own_data = ar_data__shallow_copy(result);
    if (!own_data) {
        // Handle error - cannot copy nested containers
    }
}

// Pattern 2: Destroy only if owned
if (ar_data__take_ownership(data, evaluator)) {
    ar_data__drop_ownership(data, evaluator);
    ar_data__destroy(data);
}
```

Extracted into reusable functions:
```c
// Replaces Pattern 1
ar_data_t *own_data = ar_data__claim_or_copy(result, evaluator);

// Replaces Pattern 2  
ar_data__destroy_if_owned(data, evaluator);
```

## Generalization
When extracting ownership patterns:
1. Identify all variations of the pattern across the codebase
2. Design functions that handle all cases safely
3. Use TDD to implement the functions with proper edge case handling
4. Replace patterns systematically, testing after each file
5. Maintain consistency even when you know the ownership state

## Implementation
TDD approach for implementing ownership utilities:
```c
// Red phase - write failing test
void test_data__claim_or_copy__with_unowned_data() {
    ar_data_t *data = ar_data__create_integer(42);
    void *owner = (void*)0x1234;
    
    ar_data_t *result = ar_data__claim_or_copy(data, owner);
    assert(result == data);  // Should return same pointer
    assert(ar_data__take_ownership(data, owner) == true);  // Should now be owned
    
    ar_data__drop_ownership(data, owner);
    ar_data__destroy(data);
}

// Green phase - implement minimal solution
ar_data_t* ar_data__claim_or_copy(ar_data_t *ref_data, void *owner) {
    if (!ref_data || !owner) return NULL;
    
    if (ar_data__take_ownership(ref_data, owner)) {
        ar_data__drop_ownership(ref_data, owner);
        return ref_data;
    }
    
    return ar_data__shallow_copy(ref_data);
}

// Refactor phase - consider edge cases and optimize
```

## Related Patterns
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
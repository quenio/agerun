# Dead Code After Mandatory Parameters

## Learning
When refactoring functions to require additional mandatory parameters, old wrapper functions with fewer parameters don't just become deprecated - they become dead code that always fails. These should be removed immediately, not left as misleading API surface.

## Importance
Dead code that always fails is worse than no code. Callers might attempt to use the simpler function signature, only to get NULL returns or failures. This wastes debugging time and creates a false sense of API completeness. After ar_interpreter added mandatory agency and delegation parameters, the old `ar_interpreter__create(log)` always returned NULL but remained in the codebase for 14 lines.

## Example
```c
// Real example from ar_interpreter.c after mandatory parameters added:

// BEFORE CLEANUP - Dead wrapper that always failed:
ar_interpreter_t* old_create(ar_log_t *ref_log) {  // EXAMPLE: Pattern showing dead wrapper
    // Called new function with NULL for required parameters
    return new_create_with_suffix(ref_log, NULL, NULL);  // EXAMPLE: Always returned NULL
}

// BEFORE CLEANUP - Function with suffix that did real work:
ar_interpreter_t* new_create_with_suffix(  // EXAMPLE: Pattern showing suffixed function
    ar_log_t *ref_log,
    ar_agency_t *ref_agency,
    ar_delegation_t *ref_delegation
) {
    if (!ref_log || !ref_agency || !ref_delegation) {
        return NULL;  // Validation fails for NULL parameters
    }
    // ... actual creation logic
}

// AFTER CLEANUP - Current actual implementation (ar_interpreter.c):
ar_interpreter_t* ar_interpreter__create(
    ar_log_t *ref_log,
    ar_agency_t *ref_agency,
    ar_delegation_t *ref_delegation
) {
    if (!ref_log || !ref_agency || !ref_delegation) {
        return NULL;
    }
    // ... actual creation logic
}
```

## Generalization
After refactorings that add mandatory parameters:

1. **Identify dead wrappers**: Search for old functions that call new ones with NULL
2. **Verify they're truly dead**: Check if they can ever succeed (usually they cannot)
3. **Remove immediately**: Don't leave as "deprecated" - they're misleading
4. **Update all callers**: Find with grep, update to new signature
5. **Clean up naming**: Remove differentiating suffixes if only one version remains

Pattern detection:
```bash
# Find functions that might be dead wrappers
grep -A 3 "^ar_.*__create.*{$" modules/*.c | grep "return.*__create"
```

## Implementation
Systematic cleanup process:

```bash
# 1. Find the dead function and its callers
grep -rn "ar_interpreter__create(" modules/*.{c,h}

# 2. Verify it's truly dead by examining its logic
# Look for: calls new function with NULL for new parameters

# 3. Remove from header file
# Edit modules/ar_interpreter.h - remove old declaration

# 4. Remove from implementation
# Edit modules/ar_interpreter.c - remove old function body

# 5. Update all callers to use new signature
# This requires providing the additional parameters

# 6. Test that everything still compiles
make clean && make
```

Common scenarios where this occurs:
- Adding context parameters (ar_system_t*, ar_data_t *ref_context)
- Adding configuration parameters (ar_log_t*, ar_agency_t*)
- Adding dependency injection parameters (ar_delegation_t*, ar_methodology_t*)

## Related Patterns
- [API Suffix Cleanup After Consolidation](api-suffix-cleanup-after-consolidation.md)

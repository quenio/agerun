# API Suffix Cleanup After Consolidation

## Learning
When a function originally used a suffix to differentiate from a base version (`_with_agency`), but the base version is later removed, the suffix becomes meaningless noise and should be removed. API names should reflect current reality, not historical evolution.

## Importance
Function names communicate purpose. A suffix like `_with_agency` suggests there's a version without agency, creating confusion when no such version exists. This forces developers to check if an unsuffixed version exists, wasting time. Clean names improve API discoverability and reduce cognitive load.

## Example
```c
// Historical evolution pattern (using generic names):

// Cycle 1: Original simple API
ar_module__create(ar_log_t *ref_log);  // EXAMPLE: Simple version

// Cycle 2: Added advanced API with more parameters
ar_module__create(ar_log_t *ref_log);  // EXAMPLE: Still exists but becoming dead code
ar_module__create_with_suffix(  // EXAMPLE: New version with more parameters
    ar_log_t *ref_log,
    ar_something_t *ref_dep1,  // EXAMPLE: Generic dependency type
    ar_something_t *ref_dep2   // EXAMPLE: Generic dependency type
);

// Cycle 3: Simple API becomes dead code
// - Remove simple version
// - Remove "_with_suffix" from advanced version

// CORRECT FINAL STATE - actual ar_interpreter implementation:
ar_interpreter_t* ar_interpreter__create(
    ar_log_t *ref_log,
    ar_agency_t *ref_agency,
    ar_delegation_t *ref_delegation
);
// No suffix needed - this is THE create function

// Real usage in ar_system.c:
own_system->own_interpreter = ar_interpreter__create(
    own_system->own_log,
    own_system->own_agency,
    own_system->own_delegation
);
```

## Generalization
API suffix cleanup pattern after consolidation:

1. **Identify consolidation points**: When removing base function, suffix becomes redundant
2. **Check all suffixed functions**: Look for `_with_*`, `_ex`, `_full`, `_extended`
3. **Verify no base version exists**: If base is gone, suffix is meaningless
4. **Rename to base name**: Remove the differentiating suffix
5. **Update all callers**: Usually this is simple search-and-replace

Common suffix patterns to clean:
- `_with_context` when no contextless version exists
- `_with_agency` when all callers need agency
- `_extended` when basic version removed
- `_full` when partial version removed

Detection command:
```bash
# Find functions with common differentiating suffixes
grep -rn "^ar_.*__.*_with_" modules/*.h
grep -rn "^ar_.*__.*_extended\|_full\|_ex" modules/*.h

# For each, check if base version exists
```

## Implementation
Systematic suffix cleanup:

```bash
# 1. Find the suffixed function
grep -rn "ar_module__create_with_suffix" modules/*.{h,c}  # EXAMPLE: Generic pattern

# 2. Verify no base version exists (or that it's dead code)
grep -rn "^ar_module__create[^_]" modules/*.h  # EXAMPLE: Check for base

# 3. Update header: rename function declaration
# modules/ar_module.h:  // EXAMPLE: Generic module
# - Change: ar_module__create_with_suffix(...)  // EXAMPLE: Old name
# + Change: ar_module__create(...)  // EXAMPLE: New clean name

# 4. Update implementation: rename function definition
# modules/ar_module.c: same rename  # EXAMPLE: Implementation file

# 5. Update all callers
grep -rn "ar_module__create_with_suffix" modules/*.c  # EXAMPLE: Find callers
# Replace each occurrence with ar_interpreter__create

# 6. Verify compilation
make clean && make

# 7. Run tests to ensure behavior unchanged
make test
```

Naming guidelines after consolidation:
- If only one version exists, use base name without suffix
- If multiple versions still exist, keep differentiating suffixes
- Prefer descriptive base names over generic `_ex` or `_full`

## Related Patterns
- [Dead Code After Mandatory Parameters](dead-code-after-mandatory-parameters.md)

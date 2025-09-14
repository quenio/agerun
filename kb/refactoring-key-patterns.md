# Refactoring Key Patterns

## Learning
Essential patterns for safe and effective refactoring in AgeRun, focusing on preserving behavior while improving code structure.

## Importance
Following proven refactoring patterns prevents bugs, ensures code is actually moved (not rewritten), and maintains test validity throughout changes.

## Example
```c
// Pattern 1: Module Cohesion - Split large modules
// Before: 850-line agency module
// After: 4 focused modules (registry, store, update, core)

// Pattern 2: Code Movement Verification
// MANDATORY: Verify code is moved, not rewritten
// bash: diff -u <(sed -n '130,148p' original.c) <(sed -n '11,29p' new.c)

// Pattern 3: Instance Migration
// Before:
ar_data_t* ar_expression_evaluator__evaluate(ar_expression_ast_t *ast, ar_data_t *memory) {
    // implementation using parameters
}

// After:
typedef struct ar_expression_evaluator_s {
    ar_data_t *mut_memory;  // Now instance field
} ar_expression_evaluator_t;

ar_data_t* ar_expression_evaluator__evaluate(ar_expression_evaluator_t *self, ar_expression_ast_t *ast) {
    // implementation using self->mut_memory
}

// Pattern 4: Registry Pattern Extraction
// Extract storage responsibility to focused module while preserving API
// Before: ar_methodology with internal 2D arrays
struct ar_methodology_s {
    ar_method_t ***own_methods;       // 2D array
    int *own_method_counts;           // Version counts
    int method_name_count;            // Total methods
};

// After: Delegate to registry
struct ar_methodology_s {
    ar_log_t *ref_log;
    ar_method_registry_t *own_registry;  // All storage delegated
};

// Public API unchanged:
ar_method_t* ar_methodology__get_method_with_instance(const char *name, const char *version) {
    // Now delegates to registry internally
}

// Pattern 5: Deprecation for Safety
ar_data_t* ar_old_function(ar_data_t *param) {
    (void)param;  // Suppress warning
    return NULL;  // Safe default
    // DEPRECATED: Use ar_new_function instead
}
```

## Generalization
- **Module cohesion**: Split large modules (e.g., 850-line agency → 4 focused modules)
- **Registry pattern extraction**: Move storage to focused registry while preserving public APIs
- **Merging functions**: Move helpers first, then merge implementation (don't re-implement)
- **Instance migration**: Replace parameters with `self->field` references
- **File verification**: Check `wc -l` if content seems truncated; beware "[... rest of ...]"
- **Test preservation**: Tests define behavior - fix implementation, not tests
- **Diff verification**: MANDATORY when moving code between modules
- **Strategic analysis**: Check if modern solution exists before refactoring legacy code
- **Complexity warning**: If "simple" change touches many modules → approach is wrong
- **Complete implementations**: All cases or none - partial implementations create bugs
- **Deprecation pattern**: `(void)param; return NULL/0;` with DEPRECATED docs/comments
- **Facade redundancy**: If specialized modules log errors, facades shouldn't duplicate
- **Migration verification**: Use TDD cycles even for mechanical refactoring (test→break→fix→verify)
- **Frame migration**: Include facade update as separate TDD cycle in same plan; create evaluators upfront
- **Agent context**: Agent instructions use separate context objects, not memory as context

## Implementation
1. **Before starting**: Analyze scope and create TDD plan
2. **Move code systematically**:
   ```bash
   # Verify exact code movement
   diff -u <(sed -n 'START,ENDp' old_file) <(sed -n 'START,ENDp' new_file)
   ```
3. **Preserve behavior**: Run tests after each move
4. **Clean state recovery**: If refactoring fails, revert completely
5. **Validate changes**: Test with intentional errors after adding validation
6. **Incremental commits**: Commit logical chunks with notes on remaining issues
7. **Complete API verification**: 
   ```bash
   grep -r "old_api_pattern" .  # Check ALL clients
   ```
8. **Systematic pattern replacement**:
   ```bash
   # Search for patterns across codebase
   grep -r "take_ownership.*drop_ownership" modules/
   # Replace file by file with testing
   make specific_module_tests  # After each change
   git commit  # After each successful test
   ```
9. **Post-refactoring cleanup review**:
   ```bash
   # After major refactoring, check for obsolete modules
   grep -r "#include.*temporary_module" modules/ | grep -v "_tests.c"
   # Remove modules with no remaining usage
   ```

## Related Patterns
- [Code Movement Verification](code-movement-verification.md)
- [API Migration Completion Verification](api-migration-completion-verification.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Systematic Function Renaming Pattern](systematic-function-renaming-pattern.md)
- [Search Replace Precision](search-replace-precision.md)
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [Defensive Programming Consistency](defensive-programming-consistency.md)
- [Module Removal Checklist](module-removal-checklist.md)
- [Systematic Parameter Removal Refactoring](systematic-parameter-removal-refactoring.md)
- [Non-Functional Code Detection Pattern](non-functional-code-detection-pattern.md)
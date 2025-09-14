# Compilation-Driven Refactoring Pattern

## Learning
When performing large-scale refactoring, let the compiler guide you to find all issues rather than trying to predict changes upfront. This approach is more reliable than manual inspection and ensures nothing is missed.

## Importance
Manual inspection often misses edge cases or unexpected dependencies. The compiler provides comprehensive, accurate feedback about what needs to be fixed, making refactoring safer and more complete.

## Example
During global API removal from ar_agency:
```c
// After removing global functions, compilation revealed:
// error: '_get_global_instance' defined but not used [-Werror=unused-function]

// This led us to also remove the helper function:
static ar_agency_t* _get_global_instance() {
    // This function was only used by removed global APIs
    return g_default_agency;
}

// The compiler also found all call sites needing updates:
// ar_system.c:146: error: 'ar_agency__create_agent_with_instance' undeclared
// Led to update: ar_agency__create_agent_with_instance(mut_system->own_agency, ...)
```

## Generalization
1. Make the primary change (e.g., remove functions)
2. Compile and capture all errors
3. Fix each error systematically
4. Repeat until compilation succeeds
5. Run tests to verify behavior unchanged

## Implementation
```bash
# Workflow for compilation-driven refactoring
make clean build 2>&1 | tee build.log
grep "error:" build.log | head -10  # Focus on first batch
# Fix errors
make build 2>&1 | tee build.log
# Repeat until clean
```

## Related Patterns
- [Global to Instance API Migration](global-to-instance-api-migration.md)
- [Systematic Parameter Removal](systematic-parameter-removal-refactoring.md)
- [Refactoring Patterns Detailed](refactoring-patterns-detailed.md)
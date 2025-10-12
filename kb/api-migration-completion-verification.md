# API Migration Completion Verification

## Learning
When completing API migrations, need to check ALL client code, not just obvious dependents

## Importance
Prevents compilation failures after "complete" migrations

## Example
ar_method_evaluator_tests.c still used old ar_expression_evaluator__create(log, memory, context) API

## Generalization
Always grep entire codebase for old API patterns after interface changes

## Implementation
```bash
# After changing an API, always verify no old usage remains
grep -r "old_api_pattern" .
# Example: grep -r "ar_expression_evaluator__create.*,.*," .
```

## Related Patterns
- [API Suffix Cleanup After Consolidation](api-suffix-cleanup-after-consolidation.md)
- [Dead Code After Mandatory Parameters](dead-code-after-mandatory-parameters.md)
- [Compilation-Driven Refactoring Pattern](compilation-driven-refactoring-pattern.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
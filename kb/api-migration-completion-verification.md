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
- Interface changes require comprehensive verification
- Build failures often indicate missed client updates
- Systematic verification prevents integration issues
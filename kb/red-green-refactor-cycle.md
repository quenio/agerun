# Red-Green-Refactor Cycle

## Learning
TDD requires completing ALL three phases (Red-Green-Refactor) for each behavior before moving to the next, with no commits during the cycle

## Importance
Ensures code quality through systematic improvement and prevents accumulation of technical debt

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
1. **Red Phase**: Write failing test for ONE specific behavior
2. **Green Phase**: Write minimum code to make test pass
3. **Refactor Phase**: Improve code while keeping tests green (MANDATORY)
4. **Repeat cycle** for next behavior
5. **Commit only** after ALL cycles complete

Critical: Refactor phase is NOT optional - always look for improvements

## Related Patterns
- Test-driven development methodology
- Incremental feature development
- Code quality maintenance
- Systematic improvement processes
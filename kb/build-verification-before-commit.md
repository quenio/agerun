# Build Verification Before Commit

## Learning
Full build must pass before committing, even after "successful" refactoring

## Importance
Prevents broken builds in main branch

## Example
Tests failed due to missed API updates in ar_method_evaluator_tests.c

## Generalization
Never assume completion without full verification

## Implementation
```bash
# MANDATORY before every commit
make full-build

# If full build fails, fix ALL issues before committing
# No exceptions - broken builds block other developers
```

## Related Patterns
- "Working on my machine" is insufficient for commits
- Integration failures often reveal missed dependencies
- Comprehensive verification catches edge cases
- CI/CD relies on clean builds from main branch
# Build Verification Before Commit

## Learning
Clean build must pass before committing, even after "successful" refactoring

## Importance
Prevents broken builds in main branch

## Example
Tests failed due to missed API updates in ar_method_evaluator_tests.c

## Generalization
Never assume completion without full verification

## Implementation
```bash
# MANDATORY before every commit
make clean build
make check-logs  # NEW: Always check logs for hidden issues!

# If clean build fails OR check-logs finds issues, fix ALL before committing
# No exceptions - broken builds or hidden failures block other developers

# For faster development iteration (not pre-commit):
make build       # builds from current state, no clean
make check-logs  # Still check logs even for quick builds
```

**CRITICAL**: Even when `make build` shows SUCCESS, always run `make check-logs` to catch:
- Assertion failures that didn't propagate
- Test failures with unexpected patterns
- Memory errors or crashes
- Important warnings or diagnostic output

This is especially important after the build system issue discovered on 2025-07-28 where test failures were being hidden.

## Related Patterns
- "Working on my machine" is insufficient for commits
- Integration failures often reveal missed dependencies
- Comprehensive verification catches edge cases
- CI/CD relies on clean builds from main branch
- [CI Debugging Artifact Upload](ci-debugging-artifact-upload.md)
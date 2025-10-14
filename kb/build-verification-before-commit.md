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
# MANDATORY before every commit (in this exact order)
make clean build     # Step 1: Generate fresh build logs
make check-logs      # Step 2: Analyze logs from step 1
make check-commands  # Step 3: Ensure command docs meet excellence standards

# CRITICAL: check-logs only works after full build, NOT after individual test runs
# If clean build fails OR check-logs finds issues OR commands < 90%, fix ALL before committing
# No exceptions - broken builds, hidden failures, or poor documentation block other developers

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
- [TDD Cycle Completion Verification Pattern](tdd-cycle-completion-verification-pattern.md)
- [Plan Document Completion Status Pattern](plan-document-completion-status-pattern.md)
- [CI Debugging Artifact Upload](ci-debugging-artifact-upload.md)
- [Quick Start Commands](quick-start-commands.md)
- [Pre-Commit Checklist Detailed](pre-commit-checklist-detailed.md)
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Build Time Reporting](build-time-reporting.md)
- [Build Log Verification Requirement](build-log-verification-requirement.md)
- [Build-Logs Relationship Principle](build-logs-relationship-principle.md)
- [CI Check-Logs Requirement](ci-check-logs-requirement.md)
- [Error Coverage Verification Before Enhancement](error-coverage-verification-before-enhancement.md)
- [Command Documentation Excellence Gate](command-documentation-excellence-gate.md)
- [Make Target Testing Discipline](make-target-testing-discipline.md)
- [Task Verification Before Execution](task-verification-before-execution.md)
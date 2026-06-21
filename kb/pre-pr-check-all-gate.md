# Pre-PR Check-All Gate

## Learning

Before opening a pull request, marking a draft pull request ready for review, or asking reviewers
to review new changes, run `make check-all` and treat any failure as a blocker.

## Importance

`make check-all` is the shared quality gate for non-runtime checks such as naming,
documentation validation, and changed C test BDD structure. If agents only run focused tests or a
feature-specific build sequence, recurring project-structure regressions can reach review before
the automated local guard has been exercised.

## Example

A pure expression-call PR added a `check-test-structure` target to prevent C test blocks from
missing `// Given`, `// When`, `// Then`, or `// Cleanup` statements. The target was wired into
`make check-all`, but future agents still needed an explicit pre-PR instruction to run the gate
before opening or requesting review on a pull request.

## Generalization

Pre-PR validation must include both:

1. Task-specific verification that proves the changed behavior works.
2. `make check-all` to run the repository's shared quality checks.

The `make check-all` gate does not replace `make build 2>&1`, `make check-logs`,
`make sanitize-tests 2>&1`, focused module tests, or any other verification required by the task.
It is an additional pre-PR gate.

## Implementation

Before PR creation or review request:

```bash
make check-all
```

If the command fails, fix the issue and rerun it before opening the PR or asking reviewers to
review the changes.

## Related Patterns

- [Git Push Verification](git-push-verification.md)
- [Pre-Commit Checklist Detailed](pre-commit-checklist-detailed.md)
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)

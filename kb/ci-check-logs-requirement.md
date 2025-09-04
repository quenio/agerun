# CI Check-Logs Requirement

## Learning
Code cannot be committed if `make check-logs` is failing because it will also fail in the CI build. This is a hard requirement - the CI pipeline runs check-logs and will reject any PR or commit that doesn't pass this verification.

## Importance
This requirement serves as a local CI compatibility gate:
- Prevents broken commits from reaching the repository
- Avoids CI pipeline failures that block other developers
- Ensures consistent code quality across the team
- Reduces debugging time by catching issues locally

## The CI Pipeline Flow

```
1. GitHub Actions triggered on push/PR
2. Checkout code
3. Run `make build 2>&1`
4. Run `make check-logs`  ← MUST PASS
5. Run additional tests
6. Build artifacts
```

If step 4 fails, the entire pipeline fails and the commit is rejected.

## When check-logs Produces Results

**CRITICAL**: `make check-logs` only produces meaningful results after a full build:
- After `make build 2>&1` or `make clean build 2>&1`
- NOT after individual test runs (`make some_test`)
- NOT after partial builds
- NOT on its own without prior build

## Local Verification Before Push

### Mandatory Pre-Push Checklist
```bash
# 1. Clean build to ensure fresh state (generates logs)
make clean build 2>&1

# 2. Check logs - MUST succeed (analyzes logs from step 1)
make check-logs

# 3. Only if check-logs passes, proceed to commit
git add -A
git commit -m "feat: your change"
git push
```

## Common CI Failures and Solutions

### Failure: Unwhitelisted Errors
```bash
# Local output from make check-logs:
ERROR in logs/run-tests.log: "ERROR: Some error message"

# Solution: Either fix the error or whitelist if intentional
# Option 1: Fix the root cause
# Option 2: Add to log_whitelist.yaml if intentional
```

### Failure: Unwhitelisted Success Messages
```bash
# Surprisingly, success messages can fail check-logs too
ERROR in logs/run-tests.log: "Successfully created parser"

# Solution: Add success indicators to whitelist
# log_whitelist.yaml:
- file: "logs/run-tests.log"
  contains: "Successfully created parser"
```

### Failure: Missing Expected Errors
```bash
# When check-logs expects an error that's not present
Expected error not found in logs/sanitize-tests.log

# Solution: Ensure test actually generates expected error
```

## Whitelist Management Strategy

### When to Whitelist
1. **Intentional test errors**: Error messages from tests verifying error handling
2. **Success indicators**: Messages that indicate successful operations
3. **Diagnostic output**: Debug messages needed for troubleshooting

### When NOT to Whitelist
1. **Real errors**: Actual problems that need fixing
2. **Memory leaks**: Never whitelist leak reports
3. **Segmentation faults**: Always fix, never whitelist
4. **Compilation errors**: Must be fixed at source

### Whitelist Entry Format
```yaml
# log_whitelist.yaml
- file: "logs/run-tests.log"
  contains: "ERROR: Expected test failure for null input"
  comment: "Intentional - testing error handling for NULL parameter"
```

## Emergency CI Fix Procedure

If a commit accidentally gets pushed with failing check-logs:

```bash
# 1. Pull the failing commit
git pull

# 2. Fix locally
make clean build 2>&1
make check-logs  # Identify what's failing

# 3. Fix the issue (code or whitelist)
vim log_whitelist.yaml  # or fix the code

# 4. Verify fix
make build 2>&1
make check-logs  # Must pass now

# 5. Commit and push the fix
git add -A
git commit -m "fix: resolve check-logs CI failure"
git push
```

## Best Practices

1. **Never force push**: If check-logs fails locally, it WILL fail in CI
2. **Check early, check often**: Run check-logs after each significant change
3. **Document whitelist entries**: Add comments explaining why something is whitelisted
4. **Review whitelist regularly**: Remove obsolete entries
5. **Fix root causes**: Prefer fixing errors over whitelisting them

## Integration with Development Workflow

```bash
# During development (TDD cycle)
make ar_module_tests 2>&1  # Run specific test
# ... make changes ...

# Before committing
make clean build 2>&1       # Full build
make check-logs             # Verify CI compatibility
# Only proceed to commit if this passes
```

## Related Patterns
- [Build-Logs Relationship Principle](build-logs-relationship-principle.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Pre-Commit Checklist Detailed](pre-commit-checklist-detailed.md)
- [GitHub Actions Debugging Workflow](github-actions-debugging-workflow.md)
- [Check-Logs Deep Analysis Pattern](check-logs-deep-analysis-pattern.md)
- [Whitelist vs Pattern Filtering](whitelist-vs-pattern-filtering.md)
- [GitHub Actions Deprecated Tool Migration](github-actions-deprecated-tool-migration.md)
- [CI Network Timeout Diagnosis](ci-network-timeout-diagnosis.md)
- [Error Coverage Verification Before Enhancement](error-coverage-verification-before-enhancement.md)
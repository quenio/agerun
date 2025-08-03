Check build logs for hidden issues that might not be caught by the build summary.

## What it does:

This command performs a two-phase analysis of build log files:

### Phase 1: Standard Checks
Detects known critical issues including:
- Assertion failures
- Segmentation faults or crashes
- Test failures that might not be properly reported
- Memory errors from sanitizers
- Thread safety issues
- Deep copy support errors
- Method loading warnings
- Unexpected test behaviors (e.g., tests expecting failure that succeed)
- Method evaluation failures
- Missing AST errors

### Phase 2: Deep Analysis (if standard checks pass)
Performs additional thorough analysis to catch edge cases:
- Scans for any ERROR/WARNING patterns that might have been missed
- Verifies test output consistency
- Checks for suspicious patterns in test-related output
- Searches for failure indicators (Cannot, Unable to, Failed to)
- Provides detailed counts and examples of any anomalies found

## Usage:

```bash
make check-logs
```

## Why it's important:

Even when `make build` shows "SUCCESS", the logs may contain:
- Assertion failures that didn't propagate to the build script
- Warnings or errors that should be addressed
- Tests that are failing silently
- Important diagnostic output

## Recommended workflow:

1. Run `make build` first
2. Always follow up with `make check-logs` to double-check
3. If issues are found, examine the specific log files in `logs/`

## Log file locations:

- `logs/run-tests.log` - Standard test execution
- `logs/sanitize-tests.log` - Tests with AddressSanitizer
- `logs/tsan-tests.log` - Tests with ThreadSanitizer
- `logs/analyze-exec.log` - Static analysis of executable
- `logs/analyze-tests.log` - Static analysis of tests

To view a specific log: `less logs/<logname>.log`
To search all logs: `grep -r 'pattern' logs/`

This check is especially important after the critical build system issue discovered on 2025-07-28 where test failures were not being properly reported ([details](../../kb/build-system-exit-code-verification.md)).
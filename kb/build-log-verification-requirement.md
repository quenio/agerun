# Build Log Verification Requirement

## Learning
Build systems often capture detailed output to log files for cleaner console output, but this can hide critical issues like assertion failures, crashes, and error messages that don't match expected patterns.

## Importance
Even when `make build` reports SUCCESS, the logs may contain:
- Assertion failures that didn't propagate to exit codes
- Memory errors or crashes
- Test failures with unexpected error patterns
- Important warnings or diagnostic output
- Tests that are being skipped or not running properly

## Example
During the 2025-07-28 session, we discovered the build system was hiding test failures. Even after fixing exit code propagation, detailed error messages and assertion failures are only visible in log files, not in the build summary.

## Generalization
- Never trust build summaries alone - always verify logs
- Create automated tools to scan logs for common issues
- Make log verification part of the standard workflow
- Document which patterns indicate problems

## Implementation
```bash
# Standard workflow - ALWAYS do both
make clean build
make check-logs

# Quick development iteration
make build
make check-logs  # Still essential!

# What check-logs looks for:
# - Assertion failures
# - Segmentation faults or crashes
# - Test failures (FAILED, failed, FAIL, fail)
# - Memory errors (AddressSanitizer, LeakSanitizer)
# - Thread safety issues (ThreadSanitizer)
```

The `make check-logs` target runs `scripts/check_logs.py` which scans all log files in the `logs/` directory for potential issues.

## Manual Log Inspection
When issues are found, examine specific logs:
```bash
# View a specific log
less logs/run-tests.log

# Search for patterns across all logs
grep -r "Assertion failed" logs/
grep -r "ERROR" logs/
```

## Related Patterns
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Test Diagnostic Output Preservation](test-diagnostic-output-preservation.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Comprehensive Output Review](comprehensive-output-review.md)
- [Intentional Test Errors Filtering](intentional-test-errors-filtering.md)
- [Log Format Variation Handling](log-format-variation-handling.md)
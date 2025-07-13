# Build Time Reporting

## Learning
Always report the build time from `make clean build` output when discussing build results, as it provides important context about build system performance.

## Importance
Helps track build performance over time, identify build system regressions, and provides context for CI/CD optimization decisions. A sudden increase in build time may indicate a problem.

## Example
```c
// In commit workflows or session summaries
ar_log_t *own_log = ar_log__create();
ar_log__info(own_log, "Clean build passed (1m 3s)");
ar_log__info(own_log, "All tests pass with zero memory leaks");
ar_log__destroy(own_log);
```

## Generalization
Include build time reporting in:
- Commit workflow summaries
- CI/CD pipeline logs
- Performance regression tracking
- Session completion reports
- Build system optimization discussions

## Implementation
```bash
# Extract build time from make output
make clean build 2>&1 | grep "took"
# Output: Completed at Sun Jul 13 12:36:46 -03 2025 (took 1m 3s)

# In scripts, capture and report
BUILD_OUTPUT=$(make clean build 2>&1)
BUILD_TIME=$(echo "$BUILD_OUTPUT" | grep "took" | sed 's/.*took \(.*\))/\1/')
echo "Build completed in $BUILD_TIME"
```

## Related Patterns
- [Build Verification Before Commit](build-verification-before-commit.md)
- [CI Visibility](memory-leak-detection-workflow.md)
# Build System Exit Code Verification

## Learning
Build systems must properly propagate test failures through exit codes, not just visible output. The `|| echo` pattern in Makefiles swallows errors because echo always returns 0, making the entire command succeed even when tests fail.

## Importance
This pattern can cause CI/CD systems and automated builds to report false successes, hiding critical test failures for months. Developers may also miss failures when relying on overall status messages.

## Example
```makefile
# BAD: Error is visible but exit code is 0
test:
    @for test in $(TESTS); do \
        ./$$test || echo "ERROR: Test $$test failed"; \
    done

# GOOD: Proper error propagation
test:
    @failed=0 && for test in $(TESTS); do \
        if ! ./$$test; then \
            echo "ERROR: Test $$test failed"; \
            failed=1; \
        fi; \
    done; \
    if [ $$failed -ne 0 ]; then \
        echo "ERROR: One or more tests failed"; \
        exit 1; \
    fi
```

## Generalization
- Always track failures in shell loops and exit with non-zero at the end
- Test both visible output AND exit codes when verifying build systems
- Use `set -e` or explicit error checking in shell scripts
- Prefer make's built-in error handling over shell loops when possible

## Implementation
To verify build system behavior:
```bash
# Check exit code explicitly
make test; echo "Exit code: $?"

# ALWAYS check logs for hidden issues
make check-logs

# In CI/CD, always check return values
if ! make test; then
    echo "Build failed"
    exit 1
fi
```

**NEW REQUIREMENT**: Even with proper exit codes, the build system may hide detailed output in logs. Always run `make check-logs` after any build to catch:
- Assertion failures
- Memory errors
- Test failures with unexpected patterns
- Crashes or segfaults

This double-verification ensures no issues slip through.

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Test Diagnostic Output Preservation](test-diagnostic-output-preservation.md)
- [Makefile Environment Variable Directory Target Dependencies](makefile-environment-variable-directory-gotcha.md) - Directory target dependencies with environment variable overrides
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
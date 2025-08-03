# Quick Start Commands

## Learning

The AgeRun build system provides a comprehensive set of make targets for different development tasks, from quick verification to detailed analysis.

## Importance

Understanding the right command for each situation improves development efficiency and catches issues early. Using `2>&1` ensures all output is captured, making debugging easier.

## Example

```bash
# Primary verification before commits
make build 2>&1           # ~20 lines output, runs everything
make check-logs          # CRITICAL: Always run after build

# Individual analysis commands
make analyze-exec 2>&1   # Static analysis on executable
make analyze-tests 2>&1  # Static analysis on tests  
make run-tests 2>&1      # Run all tests with auto-rebuild
make sanitize-tests 2>&1 # Run with AddressSanitizer
make run-exec 2>&1       # Build and run main executable

# Quality checks
make check-naming        # Verify naming conventions
make check-docs          # Validate documentation
make check-all           # All code quality checks

# Utilities
make clean               # Remove all build artifacts
make add-newline FILE=<file>  # Fix missing newline
make test_name 2>&1      # Build/run specific test
```

## Generalization

Always use make targets instead of direct compilation. The `2>&1` redirects stderr to stdout, capturing warnings and errors that might otherwise be missed. Build behavior defaults to incremental - use `make clean build 2>&1` for fresh builds.

## Implementation

```bash
# Standard workflow
make clean build 2>&1    # Full clean build
echo $?                  # Verify exit code is 0
make check-logs          # Check for hidden issues

# Quick verification (incremental)
make build 2>&1
make check-logs

# Debug specific test
make ar_data_tests 2>&1  # Auto-builds dependencies
```

## Related Patterns
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Build Log Verification Requirement](build-log-verification-requirement.md)
- [Stderr Redirection Debugging](stderr-redirection-debugging.md)
# Stderr Redirection for Debugging

## Problem

Many diagnostic messages, warnings, and errors are written to stderr rather than stdout. When running commands without redirecting stderr, these critical messages can be missed, leading to confusion and wasted debugging time.

## Solution

Always redirect stderr to stdout when debugging or investigating issues:

```bash
# Capture both stdout and stderr
make some_test 2>&1

# Or save to a file
make some_test > output.log 2>&1

# Or search for specific patterns
make some_test 2>&1 | grep -i warning
```

## Examples from AgeRun

### Memory Leak Warnings

The heap tracking system writes warnings to stderr:
```c
// From ar_io_variadic.c
void ar_io__warning(const char *format, ...) {
    // ...
    fputs(buffer, stderr);  // Writes to stderr!
}
```

When running tests, memory leak warnings appear on stderr:
```bash
# Without stderr redirection - warning might be missed
make ar_executable_tests

# With stderr redirection - warning is visible
make ar_executable_tests 2>&1
# Output: Warning: 41 memory leaks detected (20753 bytes). See memory_report_ar_executable_tests.log for details.
```

### Build System Output

The build system captures both stdout and stderr in logs:
```bash
# From scripts/build.sh
make run-tests > logs/run-tests.log 2>&1
```

## Best Practices

1. **Always use `2>&1` when debugging** - You don't know what might be written to stderr
2. **Check both stdout and stderr in scripts** - Ensure complete output capture
3. **Use stderr for warnings/errors in your own code** - Follow the convention
4. **Document when functions write to stderr** - Help future developers

## Common Pitfalls

- Running tests and only seeing "All tests passed!" while missing leak warnings
- Debugging "silent" failures when errors were actually printed to stderr
- CI/CD pipelines that only capture stdout and miss critical warnings

## Related Patterns

- [Evidence-based debugging](evidence-based-debugging.md) - Capture all output for analysis
- [Comprehensive output review](comprehensive-output-review.md) - Review complete command output
- [Memory debugging](memory-debugging-comprehensive-guide.md) - Memory leak warnings go to stderr
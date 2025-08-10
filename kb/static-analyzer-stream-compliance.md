# Static Analyzer Stream Compliance

## Learning
Static analyzers flag stream operations after failed I/O as potentially undefined behavior. Even when code correctly handles errors, restructuring may be needed to satisfy analyzer requirements.

## Importance
Static analysis failures block commits and CI/CD pipelines. False positives must be addressed through code restructuring rather than suppression, maintaining both correctness and compliance.

## Example
```c
// Problem: Analyzer warns about fgets() after failed fscanf()
// Original pattern that triggers warnings:
while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    // Process line
    if (strlen(buffer) > 0) {
        break;
    }
}

// Solution: Use do-while with explicit NULL checks
char *line_result = NULL;
do {
    line_result = fgets(buffer, sizeof(buffer), fp);
    if (line_result == NULL) {
        break; /* EOF or error - explicit handling */
    }
    /* Process line */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len-1] == '\n') {
        buffer[len-1] = '\0';
        len--;
    }
    if (len > 0) {
        break;  /* Found non-blank line */
    }
} while (line_result != NULL);

/* Check result after loop */
if (line_result == NULL || buffer[0] == '\0') {
    ar_io__error("Failed to read expected data");
    fclose(fp);
    return false;
}

// For popen results, always check NULL
FILE *pipe = popen(command, "r");
if (pipe != NULL) {
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);
} else {
    buffer[0] = '0';  /* Safe default */
    buffer[1] = '\0';
}
```

## Generalization
Patterns for stream operation compliance:
1. **Explicit NULL checks**: Always check file pointer before operations
2. **Early exit on error**: Don't continue after stream errors
3. **Do-while pattern**: Makes termination conditions explicit
4. **Store operation results**: Keep return values for checking
5. **Defensive defaults**: Provide safe values when operations fail

## Implementation
```bash
# Verify static analysis passes
make clean build 2>&1 | grep -E "analyze-.*: (PASSED|FAILED)"

# Check specific warnings
make analyze-tests 2>&1 | grep "warning:"

# Common patterns to avoid:
# - Continuing loops after NULL returns
# - Using streams in indeterminate state
# - Missing NULL checks on popen/fopen
# - Assuming fscanf succeeded without checking return
```

## Related Patterns
- [Static Analysis Error Handling](static-analysis-error-handling.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Evidence-Based Debugging](evidence-based-debugging.md)
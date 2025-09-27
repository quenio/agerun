# Stdout Capture Test Pattern

## Learning
Tests can capture stdout output using pipe() and dup2() to verify that specific messages are or are not printed. This allows testing of output behavior without relying on visual inspection.

## Importance
Many functions print warnings, status messages, or debugging output to stdout. Being able to programmatically verify this output ensures that functions behave correctly and don't produce unwanted messages. This is especially important for testing that certain operations (like file loading) don't occur.

## Example
```c
static void test_no_output_during_operation(void) {
    // Save original stdout
    fflush(stdout);
    int original_stdout = dup(1);
    if (original_stdout == -1) {
        printf("FAIL: Could not duplicate stdout\n");
        assert(0);
    }
    
    // Create pipe to capture output
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        close(original_stdout);
        assert(0);
    }
    
    // Redirect stdout to pipe
    if (dup2(pipefd[1], 1) == -1) {
        close(original_stdout);
        close(pipefd[0]);
        close(pipefd[1]);
        assert(0);
    }
    close(pipefd[1]);
    
    // Perform operation that should not print
    ar_system_t *mut_system = ar_system__create();
    ar_system__init(mut_system, NULL, NULL);
    
    // Restore stdout
    fflush(stdout);
    dup2(original_stdout, 1);
    close(original_stdout);
    
    // Read captured output
    char buffer[1024] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    
    // Verify no warnings were printed
    if (strstr(buffer, "Warning:") != NULL) {
        printf("FAIL: Unexpected output: %s\n", buffer);
        assert(0);
    }
    
    // Clean up
    ar_system__destroy(mut_system);
}
```

## Generalization
The stdout capture pattern involves:
1. **Save stdout**: Use dup(1) to save original file descriptor
2. **Create pipe**: Use pipe() to create capture mechanism
3. **Redirect**: Use dup2() to redirect stdout to pipe
4. **Execute**: Run code being tested
5. **Restore**: Put stdout back to original
6. **Read**: Get captured output from pipe
7. **Verify**: Check output contains/lacks expected text

## Implementation
```c
// Reusable stdout capture helper
typedef struct {  // EXAMPLE: Helper structure for capture
    int original_stdout;
    int pipefd[2];
    char buffer[4096];
} stdout_capture_t;  // EXAMPLE: Custom type for this pattern

bool capture_stdout_begin(stdout_capture_t *capture) {  // EXAMPLE: Using helper type
    fflush(stdout);
    capture->original_stdout = dup(1);
    if (capture->original_stdout == -1) return false;
    
    if (pipe(capture->pipefd) == -1) {
        close(capture->original_stdout);
        return false;
    }
    
    if (dup2(capture->pipefd[1], 1) == -1) {
        close(capture->original_stdout);
        close(capture->pipefd[0]);
        close(capture->pipefd[1]);
        return false;
    }
    close(capture->pipefd[1]);
    return true;
}

const char* capture_stdout_end(stdout_capture_t *capture) {  // EXAMPLE: Using helper type
    fflush(stdout);
    dup2(capture->original_stdout, 1);
    close(capture->original_stdout);
    
    memset(capture->buffer, 0, sizeof(capture->buffer));
    read(capture->pipefd[0], capture->buffer, sizeof(capture->buffer) - 1);
    close(capture->pipefd[0]);
    
    return capture->buffer;
}
```

## Related Patterns
- [Stderr Redirection Debugging](stderr-redirection-debugging.md)
- [Test Diagnostic Output Preservation](test-diagnostic-output-preservation.md)
- [Test Isolation Through Strategic Commenting](test-isolation-through-commenting.md)
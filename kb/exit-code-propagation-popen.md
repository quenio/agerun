# Exit Code Propagation Through popen

## Learning
Exit codes from subprocesses get multiplied by 256 when retrieved through popen()/pclose(). The actual exit code must be extracted using WEXITSTATUS() or by dividing by 256.

## Importance
Tests checking subprocess exit codes will fail mysteriously if they expect the raw exit value. Understanding this propagation prevents incorrect assertions and debugging confusion.

## Example
```c
// Running executable through popen
FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);

// Read output...
while (fgets(line, sizeof(line), pipe) != NULL) {
    // Process output
}

// Get exit status
int exit_status = pclose(pipe);

// WRONG: Checking raw value
AR_ASSERT(exit_status == 1, "Should exit with 1");  // FAILS: exit_status is 256!

// CORRECT: Extract actual exit code
if (WIFEXITED(exit_status)) {
    int actual_exit_code = WEXITSTATUS(exit_status);
    AR_ASSERT(actual_exit_code == 1, "Should exit with 1");
}

// Alternative: Handle make's exit code transformation
// make returns 2 when target fails, which becomes 512
AR_ASSERT(exit_status == 512, "Should be 512 (make exit 2 * 256)");

// Or just check for success/failure
AR_ASSERT(exit_status == 0, "Should exit successfully");  // 0 is 0 regardless
AR_ASSERT(exit_status != 0, "Should exit with error");    // Any non-zero
```

## Generalization
Exit code handling patterns:
- **Direct execution**: exit(N) → exit code N
- **Through system()/popen()**: exit(N) → N * 256
- **Through make**: Target fails → exit(2) → 512
- **Signal termination**: Different encoding (use WIFSIGNALED)

Standard macros for proper handling:
- `WIFEXITED(status)`: True if normal exit
- `WEXITSTATUS(status)`: Extract actual exit code (0-255)
- `WIFSIGNALED(status)`: True if terminated by signal
- `WTERMSIG(status)`: Get signal number

## Implementation
```c
// Robust subprocess exit code checking
int run_subprocess_and_check(const char *command, int expected_exit) {
    FILE *pipe = popen(command, "r");
    if (!pipe) {
        return -1;
    }
    
    // Process output if needed
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printf("Output: %s", buffer);
    }
    
    int status = pclose(pipe);
    
    // Comprehensive status checking
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        fprintf(stderr, "Process terminated by signal %d\n", sig);
        return -1;
    }
    
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("Process exited with code %d\n", exit_code);
        
        if (exit_code != expected_exit) {
            fprintf(stderr, "Expected exit %d, got %d\n", expected_exit, exit_code);
            return 1;
        }
        return 0;
    }
    
    fprintf(stderr, "Unknown process termination status: %d\n", status);
    return -1;
}
```

## Related Patterns
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Integration Test Binary Execution](integration-test-binary-execution.md)
- [Test Signal Reporting Practice](test-signal-reporting-practice.md)
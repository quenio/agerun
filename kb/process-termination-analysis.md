# Process Termination Analysis

## Learning
When running subprocesses in tests, properly analyze how they terminated using WIFEXITED, WIFSIGNALED, and WEXITSTATUS macros. This distinguishes between normal exits, crashes, and signal terminations, providing better diagnostics.

## Importance
Simply checking if a process "failed" misses critical information. Knowing whether it exited normally with an error code, crashed with a segfault, or was killed by a signal helps diagnose problems quickly and accurately.

## Example
```c
static void test_subprocess_execution(void) {
    // Run subprocess and capture status
    FILE *pipe = popen("./agerun 2>&1", "r");
    assert(pipe != NULL);
    
    // Process output
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // Check output...
    }
    
    // Analyze termination properly
    int status = pclose(pipe);
    
    // Check how the process terminated
    if (WIFSIGNALED(status)) {
        // Process was terminated by a signal
        int sig = WTERMSIG(status);
        printf("FAIL: Executable terminated by signal %d", sig);
        if (sig == SIGABRT) printf(" (assertion failure)");
        else if (sig == SIGSEGV) printf(" (segmentation fault)");
        else if (sig == SIGTERM) printf(" (terminated)");
        printf("\n");
        assert(0);
    } else if (WIFEXITED(status)) {
        // Process exited normally
        int exit_code = WEXITSTATUS(status);
        if (exit_code != 0) {
            printf("FAIL: Executable exited with code %d\n", exit_code);
            assert(exit_code == 0);
        }
    } else {
        // Abnormal termination (shouldn't happen with pclose)
        printf("FAIL: Executable terminated abnormally\n");
        assert(0);
    }
}
```

## Generalization
Process status analysis:
1. **WIFSIGNALED(status)**: True if process terminated by signal
2. **WTERMSIG(status)**: Gets signal number that terminated process
3. **WIFEXITED(status)**: True if process exited normally
4. **WEXITSTATUS(status)**: Gets exit code (0-255)
5. **Common signals**: SIGABRT (6), SIGSEGV (11), SIGTERM (15)

## Implementation
```c
// Comprehensive process status reporter
void analyze_process_status(int status, const char* ref_process_name) {
    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        const char* sig_name = "unknown";
        
        switch(sig) {
            case SIGABRT: sig_name = "SIGABRT (assertion/abort)"; break;
            case SIGSEGV: sig_name = "SIGSEGV (segmentation fault)"; break;
            case SIGTERM: sig_name = "SIGTERM (terminated)"; break;
            case SIGKILL: sig_name = "SIGKILL (killed)"; break;
            case SIGBUS:  sig_name = "SIGBUS (bus error)"; break;
            case SIGILL:  sig_name = "SIGILL (illegal instruction)"; break;
        }
        
        fprintf(stderr, "%s terminated by signal %d (%s)\n", 
                ref_process_name, sig, sig_name);
                
    } else if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code != 0) {
            fprintf(stderr, "%s exited with error code %d\n", 
                    ref_process_name, code);
        }
    } else {
        fprintf(stderr, "%s terminated abnormally (status: 0x%x)\n", 
                ref_process_name, status);
    }
}
```

## Related Patterns
- [Integration Test Binary Execution](integration-test-binary-execution.md)
- [Build System Exit Code Verification](build-system-exit-code-verification.md)
- [Test Diagnostic Output Preservation](test-diagnostic-output-preservation.md)
# Integration Test Binary Execution

## Learning
Integration tests can verify complete executable behavior by running the actual compiled binary and capturing its output via popen(). This tests the full system integration rather than individual units.

## Importance
Unit tests verify components in isolation, but integration tests ensure the complete executable works correctly with all components integrated. This catches issues that unit tests miss, such as initialization order, resource conflicts, and system-level behavior.

## Example
```c
// Integration test that runs actual executable
static void test_single_session(void) {
    // Verify test environment
    char cwd[1024];
    assert(getcwd(cwd, sizeof(cwd)) != NULL);
    assert(strstr(cwd, "/bin/") != NULL);  // Must run from bin directory
    
    // Always rebuild to test latest code
    int build_result = system("cd ../.. && make -s run_exec_lib > /dev/null 2>&1");
    assert(build_result == 0);
    
    // Run executable and capture output
    FILE *pipe = popen("cd ../run-exec && ./agerun 2>&1", "r");
    assert(pipe != NULL);
    
    // Read and verify output
    char buffer[512];
    bool found_error = false;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (strstr(buffer, "Starting new runtime session") != NULL) {
            found_error = true;  // Should not have second session
        }
    }
    
    // Check process termination
    int status = pclose(pipe);
    if (WIFSIGNALED(status)) {
        printf("Process terminated by signal %d\n", WTERMSIG(status));
        assert(0);
    }
    assert(WEXITSTATUS(status) == 0);
    assert(!found_error);
}
```

## Generalization
Integration testing pattern:
1. **Verify environment**: Check working directory and prerequisites
2. **Build fresh binary**: Always rebuild to test latest code
3. **Execute via popen()**: Capture both stdout and stderr with "2>&1"
4. **Parse output**: Search for expected/unexpected patterns
5. **Verify termination**: Check exit codes and signals properly

## Implementation
```c
// Helper function for integration tests
bool run_and_verify_executable(const char* ref_expected, const char* ref_unexpected) {
    // Clean environment
    system("rm -f methodology.agerun agency.agerun 2>/dev/null");
    
    // Build fresh
    if (system("make -s executable > /dev/null 2>&1") != 0) {
        return false;
    }
    
    // Run and capture
    FILE *pipe = popen("./executable 2>&1", "r");
    if (!pipe) return false;
    
    char buffer[1024];
    bool found_expected = (ref_expected == NULL);
    bool found_unexpected = false;
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (ref_expected && strstr(buffer, ref_expected)) {
            found_expected = true;
        }
        if (ref_unexpected && strstr(buffer, ref_unexpected)) {
            found_unexpected = true;
        }
    }
    
    int status = pclose(pipe);
    return found_expected && !found_unexpected && WEXITSTATUS(status) == 0;
}
```

## Related Patterns
- [Test Build Before Run Pattern](test-build-before-run-pattern.md)
- [Process Termination Analysis](process-termination-analysis.md)
- [Test Working Directory Verification](test-working-directory-verification.md)
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
# Test Build Before Run Pattern

## Learning
Integration tests should always rebuild the executable before running it to ensure they're testing the latest code changes. This prevents testing stale binaries that don't reflect current implementation.

## Importance
Without rebuilding, tests may pass against old binaries while the current code is broken, or fail against old binaries when the current code is fixed. This leads to confusion, wasted debugging time, and incorrect test results.

## Example
```c
static void test_executable_behavior(void) {
    // WRONG: Assumes executable is current
    // FILE *pipe = popen("./agerun 2>&1", "r");
    
    // CORRECT: Always rebuild first
    printf("Building executable to ensure latest version...\n");
    
    // Build the library
    int build_lib = system("cd ../.. && make -s run_exec_lib > /dev/null 2>&1");
    if (build_lib != 0) {
        printf("FAIL: Library build failed with code %d\n", build_lib);
        assert(0);
    }
    
    // Build the executable
    int build_exe = system("cd ../.. && gcc-13 -Wall -Wextra -Werror -std=c11 "
                          "-I./modules -g -O0 -o bin/run-exec/agerun "
                          "modules/ar_executable.c bin/run-exec/libagerun.a -lm 2>/dev/null");
    if (build_exe != 0) {
        printf("FAIL: Executable build failed with code %d\n", build_exe);
        assert(0);
    }
    
    // Now run the freshly built executable
    FILE *pipe = popen("cd ../run-exec && ./agerun 2>&1", "r");
    // ... rest of test
}
```

## Generalization
Build-before-run principles:
1. **Never assume currency**: Executable state is unpredictable
2. **Build explicitly**: Include build step in test setup
3. **Handle build failures**: Detect and report build errors clearly
4. **Use consistent flags**: Match production build configuration
5. **Suppress noise**: Redirect build output unless debugging

## Implementation
```c
// Reusable build verification function
bool ensure_executable_current(const char* ref_target) {
    char build_cmd[256];
    
    // Build library first
    snprintf(build_cmd, sizeof(build_cmd), 
             "make -s %s_lib > /dev/null 2>&1", ref_target);
    if (system(build_cmd) != 0) {
        return false;
    }
    
    // Build executable
    snprintf(build_cmd, sizeof(build_cmd),
             "make -s %s > /dev/null 2>&1", ref_target);
    return system(build_cmd) == 0;
}

// Use in tests
void test_feature(void) {
    assert(ensure_executable_current("run_exec"));
    // Now safe to run executable
}
```

## Related Patterns
- [Integration Test Binary Execution](integration-test-binary-execution.md)
- [Make-Only Test Execution](make-only-test-execution.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
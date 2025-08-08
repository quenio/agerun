# Test Working Directory Verification

## Learning
Tests should verify they're running from the expected working directory before proceeding, especially when using relative paths. This prevents confusing failures from wrong directory context.

## Importance
Tests often rely on relative paths to find executables, test data, or output directories. Running from the wrong directory causes mysterious "file not found" errors that waste debugging time. Early directory verification provides clear, immediate feedback.

## Example
```c
static void test_with_directory_verification(void) {
    // First verify we're running from the correct directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("FAIL: Could not get current directory\n");
        assert(0);
    }
    
    // Check if we're in expected bin subdirectory
    if (strstr(cwd, "/bin/") == NULL) {
        printf("FAIL: Test must be run from bin directory, but running from: %s\n", cwd);
        assert(0);
    }
    printf("Running from: %s\n", cwd);
    
    // Now safe to use relative paths
    if (access("../run-exec", F_OK) != 0) {
        printf("FAIL: Directory ../run-exec does not exist\n");
        assert(0);
    }
    
    // Proceed with test using verified relative paths
    FILE *pipe = popen("cd ../run-exec && ./agerun 2>&1", "r");
    // ... rest of test
}
```

## Generalization
Directory verification strategy:
1. **Get current directory**: Use getcwd() to determine location
2. **Validate expectations**: Check for expected path components
3. **Fail fast with context**: Provide clear error showing actual vs expected
4. **Verify relative targets**: Check that relative paths resolve correctly
5. **Document assumptions**: Comment why specific directory is required

## Implementation
```c
// Reusable directory verification
bool verify_test_directory(const char* ref_expected_component) {
    char cwd[PATH_MAX];
    
    // Get current directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "ERROR: Cannot determine current directory\n");
        return false;
    }
    
    // Check for expected component
    if (strstr(cwd, ref_expected_component) == NULL) {
        fprintf(stderr, "ERROR: Must run from directory containing '%s'\n", 
                ref_expected_component);
        fprintf(stderr, "Current directory: %s\n", cwd);
        return false;
    }
    
    return true;
}

// Use at test start
void test_function(void) {
    assert(verify_test_directory("/bin/"));
    // Safe to proceed with relative paths
}
```

## Related Patterns
- [Integration Test Binary Execution](integration-test-binary-execution.md)
- [Make-Only Test Execution](make-only-test-execution.md)
- [Test Isolation Through Commenting](test-isolation-through-commenting.md)
- [Parallel Test Isolation with Process-Based Resource Copying](parallel-test-isolation-process-resources.md) - Resource isolation for parallel test execution
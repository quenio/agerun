# Test File Cleanup Pattern

## Learning
Tests must proactively remove persisted files before execution to ensure isolation. Leftover files from previous test runs can cause interference and false failures.

## Importance
Without pre-test cleanup, tests become order-dependent and non-deterministic. A test that passes in isolation may fail when run after another test that leaves files behind.

## Example
```c
// Problem: Test finds existing agerun.methodology from previous run
static void test_loading_methods_from_directory(ar_executable_fixture_t *mut_fixture) {
    printf("Testing executable loads methods from directory...\n");
    
    // Solution: Remove any existing methodology file BEFORE test
    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    if (build_dir) {
        char methodology_path[512];
        snprintf(methodology_path, sizeof(methodology_path), "%s/agerun.methodology", build_dir);
        remove(methodology_path);  // Clean slate before test
    }
    
    // Now test can run without interference
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    // ... rest of test
}

// Another example: Deleting method files to test persistence loading
static void test_executable__loads_persisted_methodology(ar_executable_fixture_t *mut_fixture) {
    // Step 1: Run to create methodology file
    // Step 2: Delete source files to force load from persisted
    char command[1024];
    snprintf(command, sizeof(command), "rm -f %s/*.method", own_methods_dir);
    int rm_result = system(command);
    AR_ASSERT(rm_result == 0, "Should delete method files successfully");
    
    // Step 3: Verify no .method files remain
    snprintf(command, sizeof(command), "ls %s/*.method 2>/dev/null | wc -l", own_methods_dir);
    FILE *count_pipe = popen(command, "r");
    // ... verify count is 0
}
```

## Generalization
Pre-test cleanup strategy:
1. **Identify persistent state**: Files, directories, databases, etc.
2. **Clean before, not after**: Cleanup at test start ensures known state
3. **Verify cleanup succeeded**: Check that files are actually gone
4. **Use fixture helpers**: Centralize cleanup in test fixtures
5. **Document dependencies**: Make file dependencies explicit in tests

## Implementation
```c
// Common cleanup patterns
void cleanup_test_files(const char *test_dir) {
    // Remove specific files
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/agerun.methodology", test_dir);
    remove(path);
    
    // Remove pattern-matched files
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "rm -f %s/*.tmp %s/*.bak", test_dir, test_dir);
    system(cmd);
    
    // Verify cleanup
    struct stat st;
    if (stat(path, &st) == 0) {
        fprintf(stderr, "WARNING: Failed to remove %s\n", path);
    }
}

// In test setup
void test_setup(void) {
    cleanup_test_files(get_test_dir());
    // Now start with clean state
}
```

## Related Patterns
- [Parallel Test Isolation Process Resources](parallel-test-isolation-process-resources.md)
- [Test Isolation Through Commenting](test-isolation-through-commenting.md)
- [Method Test Template](method-test-template.md)
- [Test Working Directory Verification](test-working-directory-verification.md)
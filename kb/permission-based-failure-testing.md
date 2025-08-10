# Permission-Based Failure Testing

## Learning
Use filesystem permissions (chmod) to create predictable failure conditions for testing error handling paths. This technique allows testing scenarios that are otherwise difficult to trigger reliably.

## Importance
Error handling code often goes untested because it's hard to trigger failure conditions. Permission-based testing provides a reliable, portable way to force specific failures like write errors, ensuring error paths are properly tested.

## Example
```c
// Testing save failure handling
static void test_executable__continues_on_save_failure(ar_executable_fixture_t *mut_fixture) {
    const char *build_dir = ar_executable_fixture__get_build_dir(mut_fixture);
    char methodology_path[512];
    snprintf(methodology_path, sizeof(methodology_path), "%s/agerun.methodology", build_dir);
    
    // Create a read-only file to force save failure
    FILE *fp = fopen(methodology_path, "w");
    if (fp) {
        fprintf(fp, "This file is read-only\n");
        fclose(fp);
        
        // Make file read-only - write will fail
        chmod(methodology_path, 0444);  // r--r--r--
    }
    
    // Run test - save operation will fail due to permissions
    FILE *pipe = ar_executable_fixture__build_and_run(mut_fixture, own_methods_dir);
    
    // ... verify graceful failure handling ...
    
    // Clean up - restore permissions before removal
    chmod(methodology_path, 0644);  // rw-r--r--
    remove(methodology_path);
}
```

## Generalization
Permission-based testing scenarios:
- **Write failures**: chmod 0444 (read-only) on target file
- **Read failures**: chmod 0000 (no permissions) on source file  
- **Directory access**: chmod 0555 (no write) on directory
- **Creation failures**: chmod 0555 on parent directory

Best practices:
1. Always restore permissions in cleanup
2. Check if operation succeeded before changing permissions
3. Use specific permissions rather than removing all (more realistic)
4. Document why permissions are being changed

## Implementation
```c
// Helper for permission-based testing
typedef struct {  // EXAMPLE: Hypothetical struct for teaching
    char path[1024];
    mode_t original_mode;
    bool changed;
} permission_test_t;  // EXAMPLE: Not a real AgeRun type

// Set up failure condition
bool setup_permission_failure(void *test, const char *path, mode_t fail_mode) {  // EXAMPLE: Simplified
    // Get current permissions
    struct stat st;
    if (stat(path, &st) != 0) {
        // File doesn't exist - create it if needed
        FILE *fp = fopen(path, "w");
        if (!fp) return false;
        fclose(fp);
        stat(path, &st);
    }
    
    // Set failure permissions
    if (chmod(path, fail_mode) == 0) {
        return true;
    }
    return false;
}

// Usage in test with real AgeRun functions
void test_write_failure(void) {
    // Force write failure
    chmod("output.txt", 0444);  // Make read-only
    
    // Test with real AgeRun function
    FILE *fp = NULL;
    ar_file_result_t result = ar_io__open_file("output.txt", "w", &fp);
    AR_ASSERT(result != AR_FILE_RESULT__SUCCESS, "Should fail to write");
    
    // Always cleanup
    chmod("output.txt", 0644);  // Restore permissions
}
```

## Related Patterns
- [Test Effectiveness Verification](test-effectiveness-verification.md)
- [Dynamic Test Resource Allocation](dynamic-test-resource-allocation.md)
- [Integration Test Binary Execution](integration-test-binary-execution.md)
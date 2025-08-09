# Dynamic Test Resource Allocation

## Learning
Test fixtures should use dynamic memory allocation for resources like paths and buffers rather than static arrays, enabling proper ownership transfer and avoiding limitations of fixed-size buffers.

## Importance
- Static buffers in fixtures limit flexibility and reusability
- Dynamic allocation enables proper ownership semantics
- Allows callers to manage resource lifecycle
- Prevents buffer overflow with variable-length paths
- Follows AgeRun's ownership conventions consistently

## Example
```c
// WRONG: Static buffer in fixture limits flexibility
struct ar_executable_fixture_s {
    char methods_dir[256];  // BAD: Static buffer, can't transfer ownership
};

char* ar_executable_fixture__create_methods_dir(ar_executable_fixture_t *mut_fixture) {
    snprintf(mut_fixture->methods_dir, sizeof(mut_fixture->methods_dir),
             "/tmp/agerun_test_%d_methods", getpid());
    return mut_fixture->methods_dir;  // BAD: Returning pointer to internal buffer
}

// CORRECT: Dynamic allocation with ownership transfer
char* ar_executable_fixture__create_methods_dir(ar_executable_fixture_t *mut_fixture) {
    // Allocate memory for the path
    char *own_methods_dir = AR__HEAP__MALLOC(256, "Methods directory path");
    if (!own_methods_dir) {
        return NULL;
    }
    
    // Build the path
    pid_t pid = getpid();
    snprintf(own_methods_dir, 256, "/tmp/agerun_test_%d_methods", (int)pid);
    
    // Set up the directory
    char setup_cmd[512];
    snprintf(setup_cmd, sizeof(setup_cmd),
        "rm -rf %s 2>/dev/null && "
        "mkdir -p %s && "
        "cp methods/* %s/ 2>/dev/null",
        own_methods_dir, own_methods_dir, own_methods_dir);
    
    if (system(setup_cmd) != 0) {
        AR__HEAP__FREE(own_methods_dir);
        return NULL;
    }
    
    return own_methods_dir;  // Ownership transferred to caller
}

// Corresponding destroy function takes ownership
void ar_executable_fixture__destroy_methods_dir(ar_executable_fixture_t *mut_fixture,
                                                char *own_methods_dir) {
    if (!own_methods_dir) {
        return;
    }
    
    // Clean up directory
    char cleanup_cmd[512];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s 2>&1", own_methods_dir);
    system(cleanup_cmd);
    
    // Free the allocated memory
    AR__HEAP__FREE(own_methods_dir);  // Takes ownership and frees
}
```

## Generalization
Dynamic allocation patterns for test resources:
1. **Allocate on Creation**: Use AR__HEAP__MALLOC when creating resources
2. **Transfer Ownership**: Return owned pointers that caller must manage
3. **Document Ownership**: Use `own_` prefix and comments
4. **Provide Destroy Functions**: Match create with destroy functions
5. **Handle NULL**: Check allocations and handle failures gracefully
6. **Avoid Static Buffers**: Don't use fixed arrays in fixture structs
7. **Enable Multiple Instances**: Dynamic allocation allows multiple concurrent uses

## Implementation
```c
// Test can manage multiple resources independently
ar_executable_fixture_t *own_fixture = ar_executable_fixture__create();

// Create multiple method directories dynamically
char *own_methods_dir1 = ar_executable_fixture__create_methods_dir(own_fixture);
char *own_methods_dir2 = ar_executable_fixture__create_methods_dir(own_fixture);

// Each has independent ownership
AR_ASSERT(own_methods_dir1 != own_methods_dir2, "Should be different allocations");

// Clean up each independently
ar_executable_fixture__destroy_methods_dir(own_fixture, own_methods_dir1);
ar_executable_fixture__destroy_methods_dir(own_fixture, own_methods_dir2);

ar_executable_fixture__destroy(own_fixture);
```

## Related Patterns
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Management Model](../MMM.md)
- [Test Fixture Module Creation Pattern](test-fixture-module-creation-pattern.md)
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
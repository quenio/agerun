# Error Logging Instance Utilization Pattern

## Learning
When modules store an ar_log instance but don't use it for error reporting, this represents an incomplete implementation. All error conditions that return NULL or false should log descriptive error messages through the stored ar_log instance.

## Importance
Storing an ar_log instance without using it wastes memory and misses opportunities for debugging. Comprehensive error logging helps users understand failure causes without needing to debug the source code. This pattern ensures modules fully utilize their logging capabilities.

## Example
```c
// BEFORE: ar_log instance stored but not used
struct ar_yaml_reader_s {
    ar_log_t *ref_log;  // Stored but never used for errors
};

ar_data_t* ar_yaml_reader__read_file(ar_yaml_reader_t *mut_reader, const char *ref_filename) {
    if (!mut_reader || !ref_filename) {
        return NULL;  // Silent failure
    }
    
    FILE *file = fopen(ref_filename, "r");
    if (!file) {
        return NULL;  // Silent failure
    }
    // ...
}

// AFTER: Comprehensive error logging
ar_data_t* ar_yaml_reader__read_file(ar_yaml_reader_t *mut_reader, const char *ref_filename) {
    if (!mut_reader || !ref_filename) {
        if (mut_reader && mut_reader->ref_log && !ref_filename) {
            ar_log__error(mut_reader->ref_log, "NULL filename provided to YAML reader");
        }
        return NULL;
    }
    
    FILE *file = fopen(ref_filename, "r");
    if (!file) {
        if (mut_reader->ref_log) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), "Failed to open file for reading: %s", ref_filename);
            ar_log__error(mut_reader->ref_log, error_msg);
        }
        return NULL;
    }
    // ...
}
```

## Generalization
1. **Audit existing modules**: Search for modules with ar_log fields
2. **Identify error conditions**: Find all return NULL/false paths
3. **Add descriptive logging**: Log specific error reasons
4. **Format dynamic messages**: Use snprintf for file paths and values
5. **Check log availability**: Only log if mut_instance && mut_instance->ref_log
6. **Test error scenarios**: Verify messages appear in test output
7. **Whitelist test errors**: Add intentional errors to log_whitelist.yaml

## Implementation
Follow TDD cycles for adding error logging:
```c
// EXAMPLE: Hypothetical module demonstrating pattern
// RED: Write test expecting error message
TEST(module, logs_error_for_invalid_input) {
    ar_data_t *own_module = ar_data__create_map();  // EXAMPLE: Using ar_data_t as placeholder
    // module_process(own_module, NULL);  // EXAMPLE: Trigger error
    
    // Verify error was logged (check log output or use log mock)
    // AR_ASSERT_TRUE(error_was_logged);  // EXAMPLE: Check would go here
    
    ar_data__destroy(own_module);
}

// GREEN: Add error logging to implementation
// EXAMPLE: Pattern for adding error logging
// if (!ref_input) {
//     if (mut_module->ref_log) {
//         ar_log__error(mut_module->ref_log, "NULL input provided to module");
//     }
//     return false;
// }

// REFACTOR: Extract error message formatting if needed
static void _log_file_error(ar_log_t *ref_log, const char *ref_operation, const char *ref_filename) {
    if (ref_log && ref_filename) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: %s", ref_operation, ref_filename);
        ar_log__error(ref_log, error_msg);
    }
}
```

## Related Patterns
- [Module Consistency Verification Pattern](module-consistency-verification.md)
- [Global to Instance API Migration](global-to-instance-api-migration.md)
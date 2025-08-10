# Graceful Degradation Pattern

## Learning
Non-critical operations should warn and continue rather than error and exit, allowing the system to complete its primary purpose even when auxiliary operations fail.

## Importance
Systems should be resilient to non-essential failures. A failed log write, cache update, or persistence operation shouldn't prevent core functionality from completing. This improves user experience and system reliability.

## Example
```c
// Saving methodology is helpful but not critical to executable operation
// GOOD: Warn and continue pattern
printf("Saving methodology to file...\n");
if (ar_methodology__save_methods_with_instance(mut_methodology, METHODOLOGY_FILE_NAME)) {
    printf("Methodology saved to %s\n", METHODOLOGY_FILE_NAME);
} else {
    // Non-critical failure: warn but continue
    printf("Warning: Failed to save methodology to %s\n", METHODOLOGY_FILE_NAME);
}

// Continue with shutdown regardless
printf("Shutting down runtime...\n");
ar_system__shutdown_with_instance(mut_system);
ar_system__destroy(mut_system);
printf("Runtime shutdown complete\n\n");
return 0;  // Exit successfully - primary purpose completed

// BAD: Fail entire operation for non-critical issue
if (!ar_methodology__save_methods_with_instance(mut_methodology, METHODOLOGY_FILE_NAME)) {
    printf("Error: Failed to save methodology\n");
    return 1;  // BAD: Entire executable fails for auxiliary operation
}
```

## Generalization
Categorize operations by criticality:
- **Critical**: Core functionality that must succeed (e.g., agent creation, method execution)
- **Important**: Significant but recoverable (e.g., logging, caching)
- **Auxiliary**: Nice-to-have features (e.g., persistence, metrics)

Handle failures appropriately:
- Critical → Error and exit
- Important → Warn and attempt recovery
- Auxiliary → Warn and continue

## Implementation
```c
// Pattern for graceful degradation
typedef enum {  // EXAMPLE: Hypothetical enum for teaching
    OPERATION_CRITICAL,
    OPERATION_IMPORTANT,
    OPERATION_AUXILIARY
} operation_level_t;  // EXAMPLE: Not a real AgeRun type

bool perform_operation(int level, const char *description) {  // EXAMPLE: Simplified pattern
    bool success = true; /* perform actual operation */
    
    if (!success) {
        switch (level) {
            case 0:  // CRITICAL
                fprintf(stderr, "Error: %s failed\n", description);
                exit(1);
                
            case 1:  // IMPORTANT
                fprintf(stderr, "Warning: %s failed, attempting recovery\n", description);
                /* attempt recovery logic */
                break;
                
            case 2:  // AUXILIARY
                fprintf(stderr, "Warning: %s failed, continuing\n", description);
                break;
        }
    }
    return success;
}
```

## Related Patterns
- [Error Propagation Pattern](error-propagation-pattern.md)
- [No-Op Instruction Semantics](no-op-instruction-semantics.md)
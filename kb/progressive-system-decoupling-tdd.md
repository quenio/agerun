# Progressive System Decoupling via TDD

## Learning
Complex architectural transformations can be achieved safely through a series of small TDD cycles, each removing one specific responsibility from a module, allowing incremental progress with continuous validation.

## Importance
Large refactorings often fail because they attempt too much at once. By breaking the transformation into small, testable cycles, each change is validated before proceeding, preventing cascading failures and maintaining a working system throughout the process.

## Example
```c
// TDD Cycle Plan for transforming ar_executable from demo to bootstrap:

// Cycle 1: Remove second session
static void test_single_session(void) {
    // Given we run the executable
    FILE *pipe = popen("./agerun 2>&1", "r");
    
    // When we check the output
    bool found_second_session = false;
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        if (strstr(buffer, "Starting new runtime session") != NULL) {
            found_second_session = true;
        }
    }
    
    // Then there should be only one session
    AR_ASSERT(!found_second_session, "Executable should only have one session");
}

// Cycle 2: Remove auto-loading from system
static void test_no_auto_loading_on_init(void) {
    // Capture stdout to detect loading attempts
    int original_stdout = dup(1);
    // ... redirect stdout to pipe ...
    
    ar_system__init_with_instance(mut_system, NULL, NULL);
    
    // ... restore stdout and read buffer ...
    bool loading_attempted = (strstr(buffer, "Warning: Could not load") != NULL);
    AR_ASSERT(!loading_attempted, "System should NOT attempt to load files during init");
}

// Cycle 3: Remove auto-saving from system  
static void test_no_auto_saving_on_shutdown(void) {
    // ... create system with data ...
    ar_system__shutdown_with_instance(mut_system);
    
    struct stat st;
    bool files_exist = (stat("methodology.agerun", &st) == 0);
    AR_ASSERT(!files_exist, "System should NOT save files on shutdown");
}
```

## Generalization
Break large transformations into TDD cycles:
1. **Identify all responsibilities** to be changed
2. **Order by dependency** - remove dependent features first
3. **One cycle per responsibility** - never combine multiple changes
4. **Test the absence** - verify the removed behavior doesn't occur
5. **Complete each cycle fully** - Red→Green→Refactor before starting next
6. **Document the plan** - maintain a clear roadmap of all cycles

## Implementation
```c
// Step 1: Create a comprehensive plan
// TDD Cycles for System Transformation:
// 1. Remove feature A (test absence)
// 2. Remove feature B (test absence)  
// 3. Add replacement C (test presence)
// 4. Add replacement D (test presence)
// 5. Integration test of C+D

// Step 2: Execute one cycle at a time
// RED: Write test for absence/presence
static void test_feature_removed(void) {
    ar_system_t *system = ar_system__create();
    ar_system__init_with_instance(system, NULL, NULL);
    
    // Verify feature no longer happens
    bool feature_occurred = check_for_feature();
    AR_ASSERT(!feature_occurred, "Feature should be removed");
    
    ar_system__destroy(system);
}

// GREEN: Remove just enough code to pass
void ar_system__init_with_instance(...) {
    // Comment out or remove feature code
    // feature_code(); // REMOVED in Cycle X
}

// REFACTOR: Clean up, but don't add new behavior
```

## Related Patterns
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)
- [Red Green Refactor Cycle](red-green-refactor-cycle.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [Refactoring Patterns Detailed](refactoring-patterns-detailed.md)
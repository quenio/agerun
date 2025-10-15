# Test Execution Order Dependencies

## Learning

Test execution order can dramatically affect memory leak detection, especially when tests involve process forking or system-wide state. Fork tests must run before creating any system instances because child processes inherit memory allocations, leading to false positive leak reports.

## Importance

This pattern prevents:
- False positive memory leak reports
- Confusing debugging sessions
- Test failures that depend on execution order
- Memory inheritance issues in forked processes

## Example

```c
// WRONG: Fork test after system instance creation (41 leaks reported)
int main(void) {
    printf("Starting Executable Module Tests...\n");
    
    // Create system instance first
    ar_system_t *mut_system = ar_system__create();
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // ... run other tests ...
    
    // Fork test inherits all allocations from parent
    test_executable_run();  // Reports 41 memory leaks!
    
    ar_system__destroy(mut_system);
}

// CORRECT: Fork test before any allocations (0 leaks)
int main(void) {
    printf("Starting Executable Module Tests...\n");
    
    // Run fork test FIRST (before any allocations)
    test_executable_run();  // 0 memory leaks
    
    // NOW create system instance for other tests
    ar_system_t *mut_system = ar_system__create();
    ar_agency_t *mut_agency = ar_system__get_agency(mut_system);
    ar_methodology_t *mut_methodology = ar_agency__get_methodology(mut_agency);
    
    // ... run other tests ...
    
    ar_system__destroy(mut_system);
}
```

## Generalization

Test ordering principles:
1. Run process/fork tests before any allocations
2. Run system initialization tests before feature tests
3. Run cleanup tests last
4. Group tests by resource dependencies
5. Document order dependencies in test files

## Implementation

```c
// Test file structure for order dependencies
int main(void) {
    printf("Starting Test Suite...\n");
    
    // Stage 1: Process/fork tests (no allocations)
    test_process_fork();
    test_signal_handling();
    
    // Stage 2: Standalone tests (own allocations)
    test_data_structures();
    test_algorithms();
    
    // Stage 3: System tests (shared resources)
    ar_system_t *mut_system = ar_system__create();
    test_with_system(mut_system);
    test_integration(mut_system);
    ar_system__destroy(mut_system);
    
    // Stage 4: Cleanup verification
    test_all_resources_freed();
    
    printf("All tests passed!\n");
    return 0;
}

// Document order dependencies
/**
 * @note This test must run BEFORE any system initialization
 *       to avoid memory inheritance in forked process
 */
static void test_executable_run(void) {
    pid_t pid = fork();
    // ...
}
```

## Related Patterns
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)
- [Test Diagnostic Output Preservation](test-diagnostic-output-preservation.md)
- [BDD Test Structure](bdd-test-structure.md)
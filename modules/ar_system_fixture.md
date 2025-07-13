# System Fixture Module

The system fixture provides a proper abstraction for test setup and teardown operations in AgeRun system module tests. It encapsulates common test patterns for modules that require the full AgeRun runtime environment, eliminating the need for helper functions scattered across test files.

## Overview

The system fixture manages the complete lifecycle of a system module test, including:
- System state initialization and cleanup
- Method creation and registration via the methodology API
- Agent lifecycle support (though not directly managed)
- Memory leak detection and reporting
- Persistence file management
- System reset capabilities for persistence testing

This module follows Parnas design principles by hiding implementation details behind an opaque type and providing a clean, focused interface for module test management.

## Key Features

- **Opaque Type**: The `ar_system_fixture_t` type is opaque, hiding implementation details
- **Lifecycle Management**: Handles complete test setup and teardown automatically
- **Memory Safety**: Tracks memory allocations to detect leaks within tests
- **State Isolation**: Ensures clean system state between test runs
- **Method Registration**: Provides abstraction for creating and registering methods programmatically
- **System Reset**: Supports resetting system state for persistence testing

## API Reference

### Creating and Destroying Fixtures

```c
ar_system_fixture_t* ar_system_fixture__create(const char *ref_test_name);
void ar_system_fixture__destroy(ar_system_fixture_t *own_fixture);
```

Creates a new test fixture with the given name. The fixture must be destroyed when the test completes to ensure proper cleanup.

### Initializing Test Environment

```c
bool ar_system_fixture__initialize(ar_system_fixture_t *mut_fixture);
```

Initializes the test environment by:
- Shutting down any existing system state
- Cleaning up methodology and agency
- Removing persistence files
- Initializing a fresh system instance

### Registering Methods

```c
ar_method_t* ar_system_fixture__register_method(ar_system_fixture_t *mut_fixture,
                                                const char *ref_method_name,
                                                const char *ref_instructions,
                                                const char *ref_version);
```

Creates a method programmatically and registers it with the methodology. This eliminates the repeated pattern of creating methods and manually registering them. The returned method pointer is a borrowed reference that tests can use but should not destroy.

### System Reset

```c
void ar_system_fixture__reset_system(ar_system_fixture_t *mut_fixture);
```

Resets the system to a clean state, useful for tests that need to verify persistence by shutting down and reinitializing the system.

### Memory Leak Detection

```c
bool ar_system_fixture__check_memory(const ar_system_fixture_t *ref_fixture);
```

Checks if any memory leaks occurred during the test by relying on the heap module's automatic reporting.

### Fixture Information

```c
const char* ar_system_fixture__get_name(const ar_system_fixture_t *ref_fixture);
```

Returns the name of the test associated with the fixture.

## Usage Example

```c
static void test_my_system_function(void) {
    // Create fixture for this test
    ar_system_fixture_t *own_fixture = ar_system_fixture__create("test_my_function");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_system_fixture__initialize(own_fixture));
    
    // Register a test method
    ar_method_t *ref_method = ar_system_fixture__register_method(
        own_fixture, 
        "test_method", 
        "send(0, \"Hello\")",
        "1.0.0"
    );
    assert(ref_method != NULL);
    
    // Run test logic here...
    // Create agents, send messages, verify behavior
    
    // Check for memory leaks
    assert(ar_system_fixture__check_memory(own_fixture));
    
    // Clean up
    ar_system_fixture__destroy(own_fixture);
}
```

## Design Rationale

This module was created to complement the method test fixture by addressing patterns specific to system module tests:

1. **Method Registration Pattern**: Module tests frequently create methods programmatically rather than loading from files
2. **No Directory Requirements**: System module tests don't need directory verification since they don't use relative paths
3. **System Reset Support**: System module tests often need to test persistence by resetting the system
4. **Simplified Interface**: Focuses on the common patterns in system module tests without method file handling

## Memory Management

The system fixture follows the AgeRun Memory Management Model:
- Uses `own_` prefix for owned values that must be destroyed
- Uses `mut_` prefix for mutable references
- Uses `ref_` prefix for borrowed references
- Properly tracks and reports memory allocations
- Ensures all resources are cleaned up on destruction

## Differences from Method Test Fixture

While the method test fixture is designed for testing methods loaded from `.method` files, the system fixture is designed for testing system modules that require the full runtime:

- **Method Test Fixture**: Loads methods from disk, verifies directory paths, tests agent behavior
- **System Fixture**: Creates methods programmatically, tests system modules, supports system reset
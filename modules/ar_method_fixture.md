# Method Fixture Module

The method fixture module provides a proper abstraction for test setup and teardown operations in AgeRun method tests. It encapsulates common test patterns into a cohesive module, eliminating the need for helper functions scattered across test files.

## Overview

The method fixture module manages the complete lifecycle of a method test, including:
- System state initialization and cleanup
- Method file loading and registration
- Directory verification for test execution
- Memory leak detection and reporting
- Persistence file management

This module follows Parnas design principles by hiding implementation details behind an opaque type and providing a clean, focused interface for test management.

## Key Features

- **Opaque Type**: The `method_fixture_t` type is opaque, hiding implementation details
- **Lifecycle Management**: Handles complete test setup and teardown automatically
- **Memory Safety**: Tracks memory allocations to detect leaks within tests
- **State Isolation**: Ensures clean system state between test runs
- **Method Loading**: Provides abstraction for loading method files from disk

## API Reference

### Creating and Destroying Fixtures

```c
method_fixture_t* ar_method__fixture_create(const char *ref_test_name);
void ar_method__fixture_destroy(method_fixture_t *own_fixture);
```

Creates a new test fixture with the given name. The fixture must be destroyed when the test completes to ensure proper cleanup.

### Initializing Test Environment

```c
bool ar_method__fixture_initialize(method_fixture_t *mut_fixture);
```

Initializes the test environment by:
- Shutting down any existing system state
- Cleaning up methodology and agency
- Removing persistence files
- Initializing a fresh system instance

### Loading Method Files

```c
bool ar_method__fixture_load_method(method_fixture_t *mut_fixture,
                                const char *ref_method_name,
                                const char *ref_method_file,
                                const char *ref_version);
```

Loads a method file from disk and registers it with the methodology under the specified name and version.

### Directory Verification

```c
bool ar_method__fixture_verify_directory(const method_fixture_t *ref_fixture);
```

Verifies that the test is running from the correct directory (typically `bin/`). This is important for method tests that need to access method files using relative paths.

### Memory Leak Detection

```c
bool ar_method__fixture_check_memory(const method_fixture_t *ref_fixture);
```

Checks if any memory leaks occurred during the test by comparing allocation counts before and after test execution.

### Fixture Information

```c
const char* ar_method__fixture_get_name(const method_fixture_t *ref_fixture);
```

Returns the name of the test associated with the fixture.

## Usage Example

```c
static void test_my_method(void) {
    // Create fixture for this test
    method_fixture_t *own_fixture = ar_method__fixture_create("test_my_method");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method__fixture_initialize(own_fixture));
    
    // Verify we're in the correct directory (for method tests)
    assert(ar_method__fixture_verify_directory(own_fixture));
    
    // Load and register method
    assert(ar_method__fixture_load_method(own_fixture, 
                                              "mymethod", 
                                              "../methods/mymethod-1.0.0.method",
                                              "1.0.0"));
    
    // Run test logic here...
    
    // Check for memory leaks
    assert(ar_method__fixture_check_memory(own_fixture));
    
    // Clean up
    ar_method__fixture_destroy(own_fixture);
}
```

## Design Rationale

This module was created to address the anti-pattern of helper functions in test code. Helper functions often indicate missing abstractions. By creating a proper test fixture module, we:

1. **Encapsulate test lifecycle**: All setup and teardown logic is contained within the module
2. **Provide reusable abstractions**: Common test patterns are available to all tests
3. **Ensure consistency**: All tests follow the same initialization and cleanup procedures
4. **Improve maintainability**: Changes to test infrastructure only need to be made in one place
5. **Follow design principles**: The module has a single, well-defined responsibility

## Memory Management

The method fixture module follows the AgeRun Memory Management Model:
- Uses `own_` prefix for owned values that must be destroyed
- Uses `mut_` prefix for mutable references
- Uses `ref_` prefix for borrowed references
- Properly tracks and reports memory allocations
- Ensures all resources are cleaned up on destruction
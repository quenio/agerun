# Foundation Fixture Module

The foundation fixture provides a proper abstraction for test setup and teardown operations in AgeRun foundation module tests. It encapsulates common patterns for creating and managing data structures and expression contexts, eliminating repetitive code in tests for modules like data, expression, and instruction.

## Overview

The foundation fixture manages common patterns in foundation module testing:
- Expression context creation with pre-populated test data
- Data structure creation (maps, lists) with common test values
- Automatic tracking and cleanup of all created resources
- Memory leak detection support
- No system initialization requirements

This module follows Parnas design principles by hiding implementation details behind an opaque type and providing a clean, focused interface for foundation module test management.

## Key Features

- **Opaque Type**: The `foundation_fixture_t` type is opaque, hiding implementation details
- **Resource Tracking**: Automatically tracks and destroys all created data and contexts
- **Expression Context Helpers**: Simplifies creation of expression evaluation contexts
- **Test Data Builders**: Provides pre-populated test data structures
- **Memory Safety**: Ensures all resources are cleaned up properly
- **No System Dependencies**: Works without system initialization

## API Reference

### Creating and Destroying Fixtures

```c
foundation_fixture_t* ar_foundation_fixture_create(const char *ref_test_name);
void ar_foundation_fixture_destroy(foundation_fixture_t *own_fixture);
```

Creates a new test fixture with the given name. The fixture must be destroyed when the test completes to ensure proper cleanup of all tracked resources.

### Expression Context Creation

```c
expression_context_t* ar_foundation_fixture_create_expression_context(
    foundation_fixture_t *mut_fixture,
    const char *ref_expression
);
```

Creates an expression context with standard test data:
- Memory map with: name="TestAgent", count=42, value=3.14, status="active"
- Context map with: type="test", version=1
- Message map with: action="test", sender=0

```c
expression_context_t* ar_foundation_fixture_create_custom_expression_context(
    foundation_fixture_t *mut_fixture,
    data_t *mut_memory,
    const data_t *ref_context,
    const data_t *ref_message,
    const char *ref_expression
);
```

Creates an expression context with custom data maps. The fixture tracks the context but not the input maps.

**Note**: The memory parameter is mutable because expression evaluation returns direct references to memory fields. While expressions don't modify memory, the API requires mutable access for type correctness when returning these references.

### Test Data Creation

```c
data_t* ar_foundation_fixture_create_test_map(
    foundation_fixture_t *mut_fixture,
    const char *ref_name
);
```

Creates a map with test data based on the name:
- "user": username="alice", role="admin", id=123
- "config": mode="test", timeout=30, threshold=0.95
- default: test="value", number=42, decimal=3.14

```c
data_t* ar_foundation_fixture_create_empty_map(
    foundation_fixture_t *mut_fixture
);
```

Creates an empty map for tests that need to populate it manually.

```c
data_t* ar_foundation_fixture_create_test_list(
    foundation_fixture_t *mut_fixture
);
```

Creates a list with sample values: ["first", 42, 3.14]

### Resource Tracking

```c
void ar_foundation_fixture_track_data(
    foundation_fixture_t *mut_fixture,
    data_t *own_data
);

void ar_foundation_fixture_track_expression_context(
    foundation_fixture_t *mut_fixture,
    expression_context_t *own_context
);
```

Tracks resources created outside the fixture helpers for automatic cleanup.

### Utility Functions

```c
const char* ar_foundation_fixture_get_name(const foundation_fixture_t *ref_fixture);
bool ar_foundation_fixture_check_memory(const foundation_fixture_t *ref_fixture);
```

## Usage Example

### Expression Test Example

```c
static void test_arithmetic_expression(void) {
    // Create fixture for this test
    foundation_fixture_t *own_fixture = ar_foundation_fixture_create("test_arithmetic");
    assert(own_fixture != NULL);
    
    // Create expression context with standard test data
    expression_context_t *ref_ctx = ar_foundation_fixture_create_expression_context(
        own_fixture, "memory.count + 10"
    );
    assert(ref_ctx != NULL);
    
    // Evaluate expression
    data_t *own_result = ar_expression_evaluate(ref_ctx);
    assert(own_result != NULL);
    assert(ar_data_get_type(own_result) == DATA_TYPE_INTEGER);
    assert(ar_data_get_integer(own_result) == 52); // 42 + 10
    
    // Clean up result (context is tracked by fixture)
    ar_data_destroy(own_result);
    
    // Check for memory leaks
    assert(ar_foundation_fixture_check_memory(own_fixture));
    
    // Destroy fixture (cleans up context and test data)
    ar_foundation_fixture_destroy(own_fixture);
}
```

### Data Test Example

```c
static void test_map_operations(void) {
    // Create fixture for this test
    foundation_fixture_t *own_fixture = ar_foundation_fixture_create("test_map_ops");
    assert(own_fixture != NULL);
    
    // Create test map with user data
    data_t *ref_user = ar_foundation_fixture_create_test_map(own_fixture, "user");
    assert(ref_user != NULL);
    
    // Verify pre-populated values
    assert(strcmp(ar_data_get_map_string(ref_user, "username"), "alice") == 0);
    assert(ar_data_get_map_integer(ref_user, "id") == 123);
    
    // Create another map
    data_t *ref_config = ar_foundation_fixture_create_test_map(own_fixture, "config");
    assert(ref_config != NULL);
    
    // All cleanup handled by fixture
    ar_foundation_fixture_destroy(own_fixture);
}
```

## Design Rationale

This module was created to address repetitive patterns in foundation module tests:

1. **Expression Context Creation**: Expression tests create contexts 39+ times with similar patterns
2. **Data Structure Creation**: Data tests create maps 22+ times with test values
3. **Resource Management**: Ensure all test resources are properly cleaned up
4. **Consistency**: Provide consistent test data across all foundation tests
5. **Simplification**: Let tests focus on behavior rather than setup/teardown

## Memory Management

The foundation fixture follows the AgeRun Memory Management Model:
- Uses `own_` prefix for owned values that must be destroyed
- Uses `mut_` prefix for mutable references
- Uses `ref_` prefix for borrowed references
- Tracks all created resources for automatic cleanup
- Returns borrowed references from creation functions
- Takes ownership when tracking external resources

## Differences from Other Test Fixtures

- **Method Test Fixture**: For testing methods loaded from .method files with directory requirements
- **System Test Fixture**: For testing system modules requiring full runtime initialization
- **Foundation Fixture**: For testing foundation modules with common data patterns but no system dependencies

The foundation fixture is lightweight and focused on data creation patterns, making it ideal for modules that work with data structures and expressions without needing the full AgeRun runtime.
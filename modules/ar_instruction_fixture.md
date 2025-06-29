# Instruction Fixture Module

The instruction fixture provides test infrastructure specifically for the AgeRun instruction module's test suite. It encapsulates common patterns for creating and managing test agents, methods, data structures, and expression contexts, eliminating repetitive code in instruction module tests.

## Overview

The instruction fixture manages common patterns in instruction module testing:
- Test agent creation with automatic method registration
- System initialization for tests that require it
- Expression context creation with pre-populated test data
- Data structure creation (maps, lists) with common test values
- Automatic tracking and cleanup of all created resources
- Memory leak detection support

This module follows Parnas design principles by hiding implementation details behind an opaque type and providing a clean, focused interface for instruction module test management.

## Key Features

- **Opaque Type**: The `instruction_fixture_t` type is opaque, hiding implementation details
- **Agent Management**: Creates and tracks test agents with automatic cleanup
- **Method Registration**: Handles method creation and registration for tests
- **System Initialization**: Optional system initialization for tests that need it
- **Resource Tracking**: Automatically tracks and destroys all created resources
- **Expression Context Helpers**: Simplifies creation of expression evaluation contexts
- **Test Data Builders**: Provides pre-populated test data structures
- **Memory Safety**: Ensures all resources are cleaned up properly

## API Reference

### Creating and Destroying Fixtures

```c
instruction_fixture_t* ar_instruction__fixture_create(const char *ref_test_name);
void ar_instruction__fixture_destroy(instruction_fixture_t *own_fixture);
```

Creates a new test fixture with the given name. The fixture must be destroyed when the test completes to ensure proper cleanup of all tracked resources.

### Expression Context Creation

```c
expression_context_t* ar_instruction__fixture_create_expression_context(
    instruction_fixture_t *mut_fixture,
    const char *ref_expression
);
```

Creates an expression context with standard test data:
- Memory map with: name="TestAgent", count=42, value=3.14, status="active"
- Context map with: type="test", version=1
- Message map with: action="test", sender=0

```c
expression_context_t* ar_instruction__fixture_create_custom_expression_context(
    instruction_fixture_t *mut_fixture,
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
data_t* ar_instruction__fixture_create_test_map(
    instruction_fixture_t *mut_fixture,
    const char *ref_name
);
```

Creates a map with test data based on the name:
- "user": username="alice", role="admin", id=123
- "config": mode="test", timeout=30, threshold=0.95
- default: test="value", number=42, decimal=3.14

```c
data_t* ar_instruction__fixture_create_empty_map(
    instruction_fixture_t *mut_fixture
);
```

Creates an empty map for tests that need to populate it manually.

```c
data_t* ar_instruction__fixture_create_test_list(
    instruction_fixture_t *mut_fixture
);
```

Creates a list with sample values: ["first", 42, 3.14]

### Agent and System Management

```c
bool ar_instruction__fixture_init_system(
    instruction_fixture_t *mut_fixture,
    const char *ref_init_method_name,
    const char *ref_init_instructions
);
```

Initializes the system with the specified method. Must be called before creating agents if system initialization is required.

```c
int64_t ar_instruction__fixture_create_test_agent(
    instruction_fixture_t *mut_fixture,
    const char *ref_method_name,
    const char *ref_instructions
);
```

Creates a test agent with the specified method. The fixture:
- Creates and registers the method with version "1.0.0"
- Creates the agent
- Processes the wake message automatically
- Tracks the agent for cleanup

```c
int64_t ar_instruction__fixture_get_agent(const instruction_fixture_t *ref_fixture);
```

Returns the agent ID created by the fixture, or 0 if no agent was created.

### Resource Tracking

```c
void ar_instruction__fixture_track_data(
    instruction_fixture_t *mut_fixture,
    data_t *own_data
);

void ar_instruction__fixture_track_expression_context(
    instruction_fixture_t *mut_fixture,
    expression_context_t *own_context
);

void ar_instruction__fixture_track_resource(
    instruction_fixture_t *mut_fixture,
    void *own_resource,
    void (*destructor)(void*)
);
```

Tracks resources created outside the fixture helpers for automatic cleanup. The generic `track_resource` function allows tracking any resource type with a custom destructor.

### Utility Functions

```c
const char* ar_instruction__fixture_get_name(const instruction_fixture_t *ref_fixture);
bool ar_instruction__fixture_check_memory(const instruction_fixture_t *ref_fixture);
```

## Usage Examples

### Agent-Based Test Example

```c
static void test_instruction_with_agent(void) {
    // Create fixture
    instruction_fixture_t *own_fixture = ar_instruction__fixture_create("test_agent_instruction");
    assert(own_fixture != NULL);
    
    // Initialize system
    assert(ar_instruction__fixture_init_system(own_fixture, "init_method", "memory.ready = 1"));
    
    // Create test agent
    int64_t agent = ar_instruction__fixture_create_test_agent(
        own_fixture, "test_method", "memory.value := message"
    );
    assert(agent > 0);
    
    // Get agent's memory for instruction testing
    data_t *mut_memory = ar_agent__get_mutable_memory(agent);
    const data_t *ref_context = ar_agent__get_context(agent);
    
    // Create instruction context (manually, as instruction module provides this)
    instruction_context_t *own_ctx = ar_instruction__create_context(
        mut_memory, ref_context, NULL
    );
    
    // Run instruction
    bool result = ar_instruction__run(own_ctx, "memory.test := 42");
    assert(result);
    
    // Verify result
    assert(ar_data__get_map_integer(mut_memory, "test") == 42);
    
    // Clean up instruction context
    ar_instruction__destroy_context(own_ctx);
    
    // Check for memory leaks
    assert(ar_instruction__fixture_check_memory(own_fixture));
    
    // Destroy fixture (cleans up agent and system)
    ar_instruction__fixture_destroy(own_fixture);
}
```

### Expression Test Example

```c
static void test_arithmetic_expression(void) {
    // Create fixture for this test
    instruction_fixture_t *own_fixture = ar_instruction__fixture_create("test_arithmetic");
    assert(own_fixture != NULL);
    
    // Create expression context with standard test data
    expression_context_t *ref_ctx = ar_instruction__fixture_create_expression_context(
        own_fixture, "memory.count + 10"
    );
    assert(ref_ctx != NULL);
    
    // Evaluate expression
    data_t *own_result = ar_expression__evaluate(ref_ctx);
    assert(own_result != NULL);
    assert(ar_data__get_type(own_result) == DATA_TYPE_INTEGER);
    assert(ar_data__get_integer(own_result) == 52); // 42 + 10
    
    // Clean up result (context is tracked by fixture)
    ar_data__destroy(own_result);
    
    // Check for memory leaks
    assert(ar_instruction__fixture_check_memory(own_fixture));
    
    // Destroy fixture (cleans up context and test data)
    ar_instruction__fixture_destroy(own_fixture);
}
```

### Data Test Example

```c
static void test_map_operations(void) {
    // Create fixture for this test
    instruction_fixture_t *own_fixture = ar_instruction__fixture_create("test_map_ops");
    assert(own_fixture != NULL);
    
    // Create test map with user data
    data_t *ref_user = ar_instruction__fixture_create_test_map(own_fixture, "user");
    assert(ref_user != NULL);
    
    // Verify pre-populated values
    assert(strcmp(ar_data__get_map_string(ref_user, "username"), "alice") == 0);
    assert(ar_data__get_map_integer(ref_user, "id") == 123);
    
    // Create another map
    data_t *ref_config = ar_instruction__fixture_create_test_map(own_fixture, "config");
    assert(ref_config != NULL);
    
    // All cleanup handled by fixture
    ar_instruction__fixture_destroy(own_fixture);
}
```

## Design Rationale

This module was created to address repetitive patterns in instruction module tests:

1. **Agent Setup**: The pattern of creating method, registering it, creating agent, and processing wake message is repeated 8+ times
2. **Instruction Context Creation**: Creating contexts from agent memory/context is repeated 15+ times
3. **Expression Context Creation**: Expression tests create contexts 39+ times with similar patterns
4. **Data Structure Creation**: Data tests create maps 22+ times with test values
5. **Resource Management**: Ensure all test resources are properly cleaned up
6. **Consistency**: Provide consistent test setup across all instruction tests
7. **Simplification**: Let tests focus on instruction behavior rather than setup/teardown

## Memory Management

The instruction fixture follows the AgeRun Memory Management Model:
- Uses `own_` prefix for owned values that must be destroyed
- Uses `mut_` prefix for mutable references
- Uses `ref_` prefix for borrowed references
- Tracks all created resources for automatic cleanup
- Returns borrowed references from creation functions
- Takes ownership when tracking external resources

## Differences from Other Test Fixtures

- **Method Test Fixture**: For testing methods loaded from .method files with directory requirements
- **System Test Fixture**: For testing system modules requiring full runtime initialization
- **Instruction Fixture**: For testing the instruction module itself, providing agent creation, method registration, and data structure helpers

The instruction fixture is specifically designed for the instruction module's test suite, providing the exact patterns needed for comprehensive instruction testing including agent-based tests, expression evaluation tests, and data manipulation tests.
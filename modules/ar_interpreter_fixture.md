# AgeRun Interpreter Fixture Module

## Overview

The interpreter fixture module provides test infrastructure specifically for the interpreter module's test suite. It eliminates repetitive patterns in interpreter creation, agent setup, method registration, and instruction execution that are common across interpreter module tests.

## Purpose

Testing the interpreter module requires significant setup code including:
- Creating and managing interpreter instances
- Setting up test agents with methods
- Creating instruction contexts
- Executing instructions and verifying results
- Cleaning up resources to prevent memory leaks

The interpreter fixture encapsulates all these patterns into a clean, reusable API that makes tests more readable and maintainable.

## Key Features

### Interpreter Lifecycle Management
The fixture automatically creates an interpreter instance when the fixture is created and destroys it when the fixture is destroyed. This ensures tests always have a clean interpreter instance without manual management.

### Agent Management
The fixture provides simplified agent creation that:
- Automatically registers methods before creating agents
- Tracks all created agents for cleanup
- Processes wake messages automatically
- Provides easy access to agent memory

### Instruction Execution
The fixture offers simplified APIs for executing instructions:
- Execute instructions in an agent's context with a single function call
- Support for custom messages during execution
- Automatic context creation and cleanup
- Error handling built-in

### Resource Tracking
All resources created through the fixture are automatically tracked and cleaned up:
- Data objects created with fixture functions
- Agents created during tests
- No manual cleanup required

## API Functions

### Core Functions

#### `ar__interpreter_fixture__create`
Creates a new test fixture for interpreter tests.
- **Parameters**: Test name for identification
- **Returns**: New fixture instance
- **Note**: Automatically creates an interpreter and initializes the system

#### `ar__interpreter_fixture__destroy`
Destroys a test fixture and cleans up all resources.
- **Parameters**: Fixture to destroy
- **Note**: Automatically destroys all tracked agents and data

#### `ar__interpreter_fixture__get_interpreter`
Gets the interpreter instance managed by the fixture.
- **Parameters**: Fixture reference
- **Returns**: Borrowed reference to the interpreter
- **Note**: The fixture owns the interpreter

### Agent Functions

#### `ar__interpreter_fixture__create_agent`
Creates a test agent with the specified method.
- **Parameters**: 
  - Fixture reference
  - Method name
  - Method instructions
  - Version (optional, defaults to "1.0.0")
- **Returns**: Agent ID or 0 on error
- **Note**: Automatically registers the method and processes wake message

#### `ar__interpreter_fixture__get_agent_memory`
Gets an agent's memory for testing.
- **Parameters**: Fixture reference, agent ID
- **Returns**: Mutable reference to agent's memory
- **Note**: Returns NULL if agent not found

### Execution Functions

#### `ar__interpreter_fixture__execute_instruction`
Executes a single instruction in an agent's context.
- **Parameters**:
  - Fixture reference
  - Agent ID
  - Instruction string
- **Returns**: true on success, false on error
- **Note**: Uses the fixture's interpreter instance

#### `ar__interpreter_fixture__execute_with_message`
Executes an instruction with a custom message context.
- **Parameters**:
  - Fixture reference
  - Agent ID
  - Instruction string
  - Message data (can be NULL)
- **Returns**: true on success, false on error
- **Note**: Message is not destroyed by this function

### Helper Functions

#### `ar__interpreter_fixture__create_method`
Creates and registers a method without creating an agent.
- **Parameters**:
  - Fixture reference
  - Method name
  - Instructions
  - Version (optional)
- **Returns**: true on success
- **Note**: Useful for testing method-related functionality

#### `ar__interpreter_fixture__send_message`
Sends a message to an agent and processes it.
- **Parameters**:
  - Fixture reference
  - Target agent ID
  - Message data (ownership transferred)
- **Returns**: true if sent and processed successfully
- **Note**: Takes ownership of the message

#### `ar__interpreter_fixture__create_test_map`
Creates a test data map with common values.
- **Parameters**: Fixture reference, map name
- **Returns**: Borrowed reference to created map
- **Note**: Map contains: name, count (42), value (3.14), flag (1)

#### `ar__interpreter_fixture__track_data`
Tracks a data object for automatic cleanup.
- **Parameters**: Fixture reference, data object (ownership transferred)
- **Note**: Use for data created outside fixture helpers

## Usage Examples

### Basic Test Structure
```c
static void test_interpreter_functionality(void) {
    // Given a test fixture
    interpreter_fixture_t *own_fixture = ar__interpreter_fixture__create("test_name");
    assert(own_fixture != NULL);
    
    // When creating an agent
    int64_t agent_id = ar__interpreter_fixture__create_agent(
        own_fixture,
        "test_method",
        "memory.x := 1",
        NULL  // Use default version
    );
    assert(agent_id > 0);
    
    // And executing an instruction
    bool result = ar__interpreter_fixture__execute_instruction(
        own_fixture,
        agent_id,
        "memory.y := memory.x + 1"
    );
    assert(result == true);
    
    // Then verify the result
    data_t *mut_memory = ar__interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    data_t *ref_y = ar_data__get_map_data(mut_memory, "y");
    assert(ar_data__get_integer(ref_y) == 2);
    
    // Cleanup is automatic
    ar__interpreter_fixture__destroy(own_fixture);
}
```

### Testing with Messages
```c
static void test_message_handling(void) {
    interpreter_fixture_t *own_fixture = ar__interpreter_fixture__create("test_message");
    
    // Create agent that processes messages
    int64_t agent_id = ar__interpreter_fixture__create_agent(
        own_fixture,
        "echo_method",
        "memory.echo := message",
        "1.0.0"
    );
    
    // Execute with a custom message
    data_t *own_message = ar_data__create_string("Hello");
    bool result = ar__interpreter_fixture__execute_with_message(
        own_fixture,
        agent_id,
        "memory.result := message",
        own_message
    );
    ar_data__destroy(own_message);
    
    assert(result == true);
    
    // Verify message was used
    data_t *mut_memory = ar__interpreter_fixture__get_agent_memory(own_fixture, agent_id);
    data_t *ref_result = ar_data__get_map_data(mut_memory, "result");
    assert(strcmp(ar_data__get_string(ref_result), "Hello") == 0);
    
    ar__interpreter_fixture__destroy(own_fixture);
}
```

### Resource Tracking
```c
static void test_resource_tracking(void) {
    interpreter_fixture_t *own_fixture = ar__interpreter_fixture__create("test_tracking");
    
    // Create test data that fixture will track
    data_t *ref_map = ar__interpreter_fixture__create_test_map(own_fixture, "test");
    assert(ref_map != NULL);
    
    // Create additional data and track it
    data_t *own_list = ar_data__create_list();
    ar__interpreter_fixture__track_data(own_fixture, own_list);
    // Ownership transferred - fixture will destroy it
    
    // All resources cleaned up automatically
    ar__interpreter_fixture__destroy(own_fixture);
}
```

## Design Decisions

### Based on Interpreter API
Unlike the instruction fixture which provides expression contexts and parsing helpers, the interpreter fixture focuses on execution. It uses the interpreter module's public API rather than duplicating functionality.

### Automatic System Management
The fixture handles system initialization and cleanup automatically. Tests don't need to worry about persistence files or system state.

### Simplified Execution
The fixture provides simplified execution functions that handle context creation, execution, and cleanup in one call. This reduces boilerplate significantly.

### Memory Safety
All resources created through or tracked by the fixture are automatically cleaned up when the fixture is destroyed. This prevents memory leaks in tests.

## Dependencies

The interpreter fixture depends on:
- **ar_interpreter**: For interpreter creation and instruction execution
- **ar_instruction**: For instruction context creation
- **ar_agency**: For agent management
- **ar_methodology**: For method registration
- **ar_system**: For system initialization and message processing
- **ar_data**: For data structure creation
- **ar_list**: For tracking resources
- **ar_heap**: For memory allocation

## Comparison with Instruction Fixture

While both fixtures provide test infrastructure, they serve different purposes:

| Aspect | Instruction Fixture | Interpreter Fixture |
|--------|-------------------|-------------------|
| Focus | Parsing and AST generation | Execution and runtime behavior |
| Main API | Expression contexts, test data | Agent creation, instruction execution |
| Usage | Testing parsing logic | Testing execution logic |
| Dependencies | Minimal (data, expression) | Full system (interpreter, agency, methodology) |

## Memory Management

The interpreter fixture follows the AgeRun Memory Management Model:
- Takes ownership of data passed to `track_data` and `send_message`
- Returns borrowed references from creation functions
- Automatically destroys all tracked resources
- Uses heap tracking macros for allocation

## Error Handling

The fixture provides basic error handling:
- Creation functions return NULL or 0 on error
- Execution functions return boolean success indicators
- No detailed error messages (tests should verify specific behaviors)

## Future Enhancements

Potential improvements for the fixture:
- Error message capture for testing error cases
- Batch instruction execution
- Performance measurement helpers
- Multi-agent test scenarios
- Method versioning test helpers
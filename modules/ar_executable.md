# AgeRun Executable Module

## Overview

The executable module serves as the main entry point for the AgeRun runtime system. It provides a demonstration application that showcases the complete functionality of the AgeRun agent system, including agent creation, message passing, method execution, and persistence.

## Purpose

The executable module has multiple purposes:

1. **Primary Entry Point**: Provides the `main()` function that initializes and runs the system
2. **Example Application**: Demonstrates proper usage of the AgeRun API
3. **Integration Test**: Tests the complete system including persistence across sessions
4. **Reference Implementation**: Shows best practices for memory management and error handling

## Key Functions

### Main Entry Point

```c
int main(int argc, char *argv[]);
```

The main function that:
- Initializes the AgeRun system with an optional initial agent
- Creates demonstration agents if persistence files don't exist
- Processes messages between agents
- Saves system state before shutdown
- Demonstrates proper cleanup and memory management

## Implementation Details

### Program Flow

1. **System Initialization**
   - Initializes the system with optional initial agent (ID 1, echo-1.0.0 method)
   - The initial agent is created only if agency persistence doesn't exist
   - Loads persisted agents and methods from files if they exist

2. **Agent Creation** (if no persistence)
   - Creates agent ID 2 using string-builder-1.0.0 method
   - Creates agent ID 3 using message-router-1.0.0 method
   - Creates agent ID 4 using agent-manager-1.0.0 method

3. **Message Demonstration**
   - Sends various messages to test agent functionality
   - Shows string concatenation (string-builder)
   - Demonstrates message routing based on content
   - Tests agent lifecycle management

4. **Persistence**
   - Saves all agents to agency.agerun
   - Saves all methods to methodology.agerun
   - On subsequent runs, loads saved state instead of creating new agents

5. **Cleanup**
   - Processes remaining messages
   - Performs orderly shutdown
   - Ensures zero memory leaks

### Message Examples

The executable sends several demonstration messages:

```c
// String builder demonstration
ar_agent__send(/* to */ 2, /* from */ 0, 
    ar_data__create_map_2("op", "concat", 
                          "values", ar_data__create_list_2("Hello", "World")));

// Message routing demonstration  
ar_agent__send(/* to */ 3, /* from */ 0,
    ar_data__create_map_2("type", "urgent",
                          "content", "Alert!"));

// Agent management demonstration
ar_agent__send(/* to */ 4, /* from */ 0,
    ar_data__create_map_3("action", "create",
                          "method", "string-builder",
                          "version", "1.0.0"));
```

## Usage Example

```bash
# First run - creates agents and saves state
./bin/agerun
# Output shows agent creation and message processing

# Second run - loads saved state
./bin/agerun  
# Output shows loaded agents continue processing
```

## Memory Management

The executable demonstrates proper memory management:

- **Message Ownership**: Messages are owned by the agent module after sending
- **Temporary Data**: All temporary data objects are properly destroyed
- **System Cleanup**: Proper shutdown ensures all resources are freed
- **Zero Leaks**: The implementation achieves zero memory leaks

## Design Principles

The executable module follows these principles:

1. **Demonstration Focus**: Code is written to be educational and clear
2. **Error Handling**: Shows graceful handling of missing files
3. **Memory Safety**: Demonstrates proper ownership and cleanup patterns
4. **Real-World Usage**: Provides a realistic example of system usage

## Dependencies

The executable module depends on:
- `ar_system`: For system initialization and message processing
- `ar_agent`: For agent creation and message sending
- `ar_data`: For creating message data structures
- Standard C library: For printf output

## Testing

While the executable itself is not unit tested (being a main entry point), it serves as an integration test for the entire system. The clean_build.sh script runs the executable and verifies:
- Successful execution
- No memory leaks
- Proper persistence functionality

## Implementation Notes

### Initial Agent

The system can be initialized with an optional initial agent. The executable demonstrates this by creating agent ID 1 with the echo method if no persistence exists. This agent:
- Receives system messages
- Can be used for system monitoring
- Demonstrates the wake/sleep lifecycle

### Persistence Behavior

The executable demonstrates two execution modes:
1. **First Run**: Creates new agents and methods, then saves state
2. **Subsequent Runs**: Loads saved state and continues operation

This shows how AgeRun supports long-running, persistent agent systems.

### Message Processing

The executable shows different message processing patterns:
- Immediate processing after each send
- Batch processing of multiple messages
- Processing until queue is empty

## See Also

- [ar_system.md](ar_system.md) - Core system runtime
- [ar_agent.md](ar_agent.md) - Agent management
- [ar_agency.md](ar_agency.md) - Agent persistence
- [ar_methodology.md](ar_methodology.md) - Method management
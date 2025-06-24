# AgeRun System Module

## Overview

The system module provides the core runtime management for the AgeRun agent system. It orchestrates the initialization, message processing, persistence, and shutdown of the entire system, serving as the central coordinator for all agent operations.

## Purpose

The system module is responsible for:

1. **System Initialization**: Setting up the runtime environment with optional initial agent
2. **Message Processing**: Managing the inter-agent message queue and processing loop
3. **Persistence Coordination**: Orchestrating save/load operations for agents and methods
4. **Interpreter Management**: Creating and managing the instruction interpreter
5. **Lifecycle Management**: Ensuring proper startup and shutdown sequences

## Architecture

The system module acts as a facade that coordinates several subsystems:

```
┌─────────────────┐
│     System      │
├─────────────────┤
│ - Initialization│
│ - Message Loop  │
│ - Persistence   │
│ - Interpreter   │
└────────┬────────┘
         │ Coordinates
    ┌────┴────┬────────┬───────────┐
    ▼         ▼        ▼           ▼
┌─────────┐┌──────┐┌──────────┐┌────────────┐
│ Agency  ││Agent ││Methodology││Interpreter │
└─────────┘└──────┘└──────────┘└────────────┘
```

## Key Functions

### Initialization and Shutdown

```c
/**
 * Initializes the AgeRun system.
 * @param initial_method_name Optional method name for initial agent (can be NULL)
 * @param initial_method_version Optional method version for initial agent (can be NULL)
 * @note If both parameters are provided, creates agent ID 1 with specified method
 */
void ar__system__init(const char *initial_method_name, const char *initial_method_version);

/**
 * Shuts down the AgeRun system.
 * @note Sends sleep messages to all agents before cleanup
 */
void ar__system__shutdown(void);
```

### Message Processing

```c
/**
 * Processes the next message in the queue.
 * @return true if a message was processed, false if queue was empty
 */
bool ar__system__process_next_message(void);

/**
 * Processes a specific number of messages.
 * @param max_messages Maximum number of messages to process
 * @return Number of messages actually processed
 */
int ar__system__process_messages(int max_messages);

/**
 * Processes all pending messages until the queue is empty.
 * @return Total number of messages processed
 */
int ar__system__process_all_messages(void);
```

### Persistence Operations

```c
/**
 * Saves the current system state.
 * @note Saves both agents (via agency) and methods (via methodology)
 */
void ar__system__save(void);

/**
 * Loads system state from persistence files.
 * @note Loads both agents and methods if files exist
 */
void ar__system__load(void);
```

### State Inspection

```c
/**
 * Checks if the system has been initialized.
 * @return true if initialized, false otherwise
 */
bool ar__system__is_initialized(void);
```

## Implementation Details

### Initialization Sequence

1. **Load Persistence**: Attempts to load saved agents and methods
2. **Create Initial Agent**: If specified and no agents exist, creates agent ID 1
3. **Initialize Interpreter**: Creates the instruction interpreter instance
4. **Send Wake Messages**: Notifies all agents they are active

### Message Processing Loop

The system maintains a central message queue that:
- Receives messages from agents via `ar__agent__send()`
- Processes messages in FIFO order
- Executes the recipient agent's method with the message
- Handles any errors gracefully

### Interpreter Integration

The system owns and manages a single interpreter instance that:
- Executes method instructions for all agents
- Maintains its own internal state
- Is properly cleaned up on shutdown

### Shutdown Sequence

1. **Send Sleep Messages**: All active agents receive `__sleep__` messages
2. **Process Final Messages**: Ensures sleep messages are delivered
3. **Save State**: Persists current system state
4. **Cleanup Resources**: Destroys interpreter and marks system uninitialized

## Usage Examples

### Basic System Lifecycle

```c
// Initialize system with an echo agent
ar__system__init("echo", "1.0.0");

// Process some messages
int processed = ar__system__process_messages(10);
printf("Processed %d messages\n", processed);

// Save state and shutdown
ar__system__save();
ar__system__shutdown();
```

### Continuous Processing

```c
// Initialize and load saved state
ar__system__init(NULL, NULL);
ar__system__load();

// Process until no more messages
int total = ar__system__process_all_messages();
printf("Processed %d total messages\n", total);

// Clean shutdown
ar__system__shutdown();
```

### Message Processing Patterns

```c
// Single message processing
if (ar__system__process_next_message()) {
    printf("Processed one message\n");
}

// Batch processing with limit
int batch_size = 100;
int processed = ar__system__process_messages(batch_size);

// Process all pending
int all = ar__system__process_all_messages();
```

## Memory Management

The system module follows strict ownership rules:

- **Interpreter Ownership**: The system owns and must destroy the interpreter
- **No Message Ownership**: Messages are owned by the agent module
- **No Agent/Method Ownership**: These are managed by agency and methodology
- **Initialization State**: Tracked by static boolean `_initialized`

## Design Principles

The system module adheres to these principles:

1. **Facade Pattern**: Provides a simple interface to complex subsystems
2. **Single Responsibility**: Focused on runtime orchestration only
3. **No Circular Dependencies**: Depends on other modules but none depend on it
4. **Information Hiding**: Internal state (interpreter, initialized flag) is private
5. **Clear Ownership**: Documents and follows ownership rules

## Dependencies

The system module depends on:
- `ar_agent`: For message queue access and agent operations
- `ar_agency`: For agent persistence operations
- `ar_methodology`: For method persistence operations
- `ar_interpreter`: For executing agent methods
- `ar_data`: For creating __sleep__ and __wake__ messages

## Error Handling

The system module handles errors gracefully:
- **Missing Files**: Continues operation if persistence files don't exist
- **Invalid Methods**: Logs errors but continues processing other messages
- **Memory Errors**: Proper cleanup even if initialization fails

## Thread Safety

The current implementation is NOT thread-safe. The system assumes:
- Single-threaded execution
- No concurrent access to message queue
- Sequential message processing

Future versions may add thread safety for scalability.

## Future Enhancements

Potential improvements identified in TODO.md:
- Split into focused modules (message_broker, runtime)
- Support for distributed agent communication
- Thread-safe message processing
- Performance monitoring and metrics
- Hot-reload of methods

## See Also

- [ar_agent.md](ar_agent.md) - Agent management and messaging
- [ar_agency.md](ar_agency.md) - Agent persistence and registry
- [ar_methodology.md](ar_methodology.md) - Method management
- [ar_interpreter.md](ar_interpreter.md) - Instruction execution